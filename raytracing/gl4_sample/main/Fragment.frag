#version 330

in vec2 fragmentTexCoord;

layout(location = 0) out vec4 fragColor;

uniform int g_screenWidth;
uniform int g_screenHeight;

uniform vec3 g_bBoxMin;
uniform vec3 g_bBoxMax;

uniform vec3 g_lightPos;

uniform mat4 g_rayMatrix;
uniform vec4 g_bgColor;

vec3 EyeRayDir(float x, float y, float w, float h)
{
    float fov = 3.141592654f/(2.0f); 
    vec3 ray_dir;

    ray_dir.x = x+0.5f - (w/2.0f);
    ray_dir.y = y+0.5f - (h/2.0f);
    ray_dir.z = -(w)/tan(fov/2.0f);

    return normalize(ray_dir);
}

float sdSphere(vec3 p, float s)
{
    return length(p)-s;
}

float sdPlane( vec3 p, vec4 n )
{
  // n must be normalized
  return dot(p,n.xyz) + n.w;
}

float sdCone( vec3 p, vec2 c )
{
    // c must be normalized
    float q = length(p.xy);
    return dot(c,vec2(q,p.z));
}

struct Sphere
{
    vec3 center;
    vec4 diffColor;
    vec4 specColor;
    float r;
    float specPower;
    float reflection;
    float refraction;
};

struct ICone
{
    vec3 center;
    vec4 diffColor;
    vec4 specColor;
    vec2 c;
    float specPower;
    float reflection;
    float refraction;
};

struct LightSource
{
    vec3 pos;
    vec4 color;
};

int type = 0; //current object type

const float eps = 0.0001; //raymarching epsilon
const float tmax = 30.0; //max length of ray
const int obj_cnt = 8;
const int sphere_cnt = 6;
const int light_cnt = 2;
const int MAX_REF_CNT = 4; //max reflected/refracted rays
const int MAX_STEP_CNT = 300;

struct LightSource lights[light_cnt];
struct Sphere spheres[sphere_cnt];
float reflection[obj_cnt];
float refraction[obj_cnt];

float EvaluateDistance(vec3 p)
{
    float res, r;
    res = sdPlane(p,normalize(vec4(0,0.5,0,1.0)));
    type = 0;
    for (int i = 0; i < sphere_cnt - 2; i++) {
        r = sdSphere(p + spheres[i].center, spheres[i].r);
        if (res >= r) {
            res = r;
            type = i + 1;
        }
    }
    //intersect two last spheres
    r = max(sdSphere(p + spheres[4].center, spheres[4].r), sdSphere(p + spheres[5].center, spheres[5].r));
    if (res >= r) {
        res = r;
        type = 5;
    }
    r = max(sdCone(p+vec3(0,0.5,3), normalize(vec2(2,-0.3))), /*sdSphere(p+vec3(-3,0.5,0),2));*/sdCone(p+vec3(0,0.5,1), normalize(vec2(2,0.3))));
    if (res >= r) {
        res = r;
        type = 5;
    }
    return res;
}

vec3 EstimateNormal(vec3 z, float eps)
{
    vec3 z1 = z + vec3(eps, 0, 0);
    vec3 z2 = z - vec3(eps, 0, 0);
    vec3 z3 = z + vec3(0, eps, 0);
    vec3 z4 = z - vec3(0, eps, 0);
    vec3 z5 = z + vec3(0, 0, eps);
    vec3 z6 = z - vec3(0, 0, eps);

    float dx = EvaluateDistance(z1) - EvaluateDistance(z2);
    float dy = EvaluateDistance(z3) - EvaluateDistance(z4);
    float dz = EvaluateDistance(z5) - EvaluateDistance(z6);

    return normalize(vec3(dx, dy, dz) / (2.0*eps));
}

vec4 Phong(vec3 n, vec3 l, vec3 v, vec3 p)
{
    vec4  diffColor;
    vec4  specColor;
    float specPower;
    if (type == 0) {
        //chess plane
        const float delta = 1.0;
        float dx = p.x / delta;
        float dz = p.z / delta;
        if (dx < 0)
            dx = -dx + 1;
        if (dz < 0)
            dz = -dz + 1;
        int i = int(dx);
        int j = int(dz);
        if ((i % 2 == 0 && j % 2 == 0) || (i % 2 == 1 && j % 2 == 1)) {
            diffColor = vec4 ( 1.0, 1.0, 1.0, 1.0 );
            specColor = vec4 ( 1.0, 1.0, 1.0, 1.0 );
        } else {
            diffColor = vec4 ( 0.0, 0.0, 0.0, 1.0 );
            specColor = vec4 ( 0.0, 0.0, 0.0, 1.0 );
        }
        specPower = 200.0;
    } else if (type > 0 && type <= sphere_cnt){
        //spheres
        diffColor = spheres[type-1].diffColor;
        specColor = spheres[type-1].specColor;
        specPower = spheres[type-1].specPower;
    } else if (type == sphere_cnt + 1) {
        diffColor = vec4(1.0);
        specColor = vec4(0.0);
        specPower = 0;
    }

    vec3 n2   = normalize ( n );
    vec3 l2   = normalize ( l );
    vec3 v2   = normalize ( v );
    vec3 r    = reflect ( -v2, n2 );
    vec4 diff = diffColor * max ( dot ( n2, l2 ),0.5 );
    vec4 spec = specColor * pow ( max ( dot ( l2, r ), 0.0 ), specPower );

    return diff + spec;
}

float SoftShadow(in vec3 ro, in vec3 rd)
{
    const float k = 4;
    float res = 1.0;
    int cnt = 0;
    float t = 0.01;
    while (cnt < 50)
    {
        float h = EvaluateDistance(ro + rd * t);
        if(h < eps)
            return 0.0;
        res = min( res, k*h/t );
        t += h;
        cnt++;
    }
    return res;
}

void InitSpheres()
{
    spheres[0].center = vec3(0.0,0.4,0.0);
    spheres[0].r = 0.4;
    spheres[0].diffColor = vec4(0.6,0.2,0.9,1);
    spheres[0].specColor = vec4(0.69,0.22,0.93,1);
    spheres[0].specPower = 10.0;
    spheres[0].reflection = 0.7;
    spheres[0].refraction = 0.0;

    spheres[1].center = vec3(0.8,0.4,0.0);
    spheres[1].r = 0.2;
    spheres[1].diffColor = vec4(0.9,0.0,0.5,1);
    spheres[1].specColor = vec4(1.0,0.07,0.57,1);
    spheres[1].specPower = 10.0;
    spheres[1].reflection = 0.7;
    spheres[1].refraction = 0.0;

    spheres[2].center = vec3(0.8,0.2,1.0);
    spheres[2].r = 0.3;
    spheres[2].diffColor = vec4(1.0,1.0,1.0,0.5);
    spheres[2].specColor = vec4(1.0,1.0,1.0,0.5);
    spheres[2].specPower = 10.0;
    spheres[2].reflection = 0.0;
    spheres[2].refraction = 0.3;

    spheres[3].center = vec3(0.4,0.4,0.6);
    spheres[3].r = 0.2;
    spheres[3].diffColor = vec4(0.49,0.49,1.0,1);
    spheres[3].specColor = vec4(0.49,0.49,1.0,1);
    spheres[3].specPower = 10.0;
    spheres[3].reflection = 0.7;
    spheres[3].refraction = 0.0;

    spheres[4].center = vec3(-0.5,-0.1,-0.7);
    spheres[4].r = 0.3;
    spheres[4].diffColor = vec4(0.1,0.7,0.9,1);
    spheres[4].specColor = vec4(0.0,0.74,1.0,1);
    spheres[4].specPower = 10.0;
    spheres[4].reflection = 0.7;
    spheres[4].refraction = 0.0;

    spheres[5].center = vec3(-0.1,-0.1,-0.7);
    spheres[5].r = 0.3;
    spheres[5].diffColor = vec4(0.1,0.7,0.9,1);
    spheres[5].specColor = vec4(0.0,0.74,1.0,1);
    spheres[5].specPower = 10.0;
    spheres[5].reflection = 0.7;
    spheres[5].refraction = 0.0;
}

void InitObjects()
{
    InitSpheres();
    reflection[0] = 0.6;
    refraction[0] = 0.0;
    reflection[sphere_cnt + 1] = -0.1;
    refraction[sphere_cnt + 1] = -0.1;
    for (int i = 0; i < sphere_cnt; i++) {
        reflection[i + 1] = spheres[i].reflection;
        refraction[i + 1] = spheres[i].refraction;
    }
}

void InitLightSources()
{
    lights[0].pos = normalize(vec3(15,10,10));
    lights[0].color = vec4(0.878, 0.117, 0.741, 1);
    lights[1].pos = normalize(vec3(15,10,15));
    lights[1].color = vec4(0.231, 0.835, 0.176, 1);
}

void main(void)
{	
    //get screen geometry
    float w = float(g_screenWidth);
    float h = float(g_screenHeight);

    // get curr pixelcoordinates
    float x = fragmentTexCoord.x*w; 
    float y = fragmentTexCoord.y*h;

    // generate initial ray
    vec3 ray_pos = vec3(0,0,0); 
    vec3 ray_dir = EyeRayDir(x,y,w,h);

    // transorm ray with matrix
    ray_pos = (g_rayMatrix*vec4(ray_pos,1)).xyz;
    ray_dir = mat3(g_rayMatrix)*ray_dir;

    //initialization
    InitObjects();
    InitLightSources();

    vec3 eye_pos = ray_pos;
    int hit = 0, cnt = 0, ref_cnt = 0, i;
    float dt, t = 0.0;

    fragColor = vec4(0,0,0,0);
    while (/*cnt < MAX_STEP_CNT &&*/ t < tmax) {
        dt = EvaluateDistance(ray_pos);
        if (dt < 0)
            break;
        if (dt < eps) {
            hit = 1;
            //shadowing
            float sh = 0.0;
            vec4 col = vec4(0,0,0,1);
            for (i = 0; i < light_cnt; i++) {
                sh += SoftShadow(ray_pos, normalize(lights[i].pos-ray_pos));
            }
            col = vec4(sh, sh, sh,1.0);
            //local coloring - Phong
            vec3 normal = EstimateNormal(ray_pos,0.0001);
            vec3 tmp = normalize(eye_pos-ray_pos);
            for (i = 0; i < light_cnt; i++) {
                col += Phong(normal,normalize(lights[i].pos - ray_pos), tmp,ray_pos);
            }
            col /= 2;
            //reflection
            if (ref_cnt >= MAX_REF_CNT)
                break;
            if (reflection[type] > 0) {
                fragColor += col * reflection[type];
                ref_cnt++;
                vec3 tmp = normal*dot(ray_dir, normal)*(-2);
                ray_dir = normalize(tmp + ray_dir);
                ray_pos += ray_dir;
                continue;
            } else if (refraction[type] > 0) {
                fragColor = mix(fragColor,col,0.5);
                ref_cnt++;
                float eta = refraction[type];
                float cos_theta = -dot(normal, ray_dir);
                if(cos_theta < 0) {
                    normal *= -1.0;
                    cos_theta *= -1.0;
                    eta = 1.0/eta;
                }
                float k = 1.0 - eta*eta*(1.0-cos_theta*cos_theta);
                if(k > 0) {
                    ray_dir = eta*ray_dir + (eta*cos_theta - sqrt(k))*normal;
                    ray_dir = normalize(ray_dir);
                }
                else {
                    break;
                }
                ray_pos += ray_dir;
                continue;
            } else {
                fragColor += col;
            }
        }
        t+=dt;
        cnt++;
        ray_pos = ray_pos + ray_dir*dt;
    }
    if (hit == 0)
        fragColor = vec4(0.0,0.0,0.0,1);
}

