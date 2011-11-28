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
    float r;
    vec4 diffColor;
    vec4 specColor;
    float specPower;
    float reflection;
    float refraction;
};

struct ISphere
{
    vec3 center1, center2;
    float r1, r2;
    vec4 diffColor;
    vec4 specColor;
    float specPower;
    float reflection;
    float refraction;
};

struct ICone
{
    vec3 center1, center2;
    vec2 c1, c2;
    vec4 diffColor;
    vec4 specColor;
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
const float tmax = 40.0; //max length of ray
const int obj_cnt = 9;
const int sphere_cnt = 6;
const int isphere_cnt = 1;
const int icone_cnt = 1;
const int light_cnt = 2;
const int MAX_REF_CNT = 3; //max reflected/refracted rays
const int MAX_CNT = 350;
const int AA_cnt = 3;

vec3 dir_eps[AA_cnt];

struct LightSource lights[light_cnt];
struct Sphere spheres[sphere_cnt];
struct ISphere ispheres[isphere_cnt];
struct ICone icones[icone_cnt];
float reflection[obj_cnt];
float refraction[obj_cnt];

float EvaluateDistance(vec3 p)
{
    float res, r;
    int i;
    res = sdPlane(p,normalize(vec4(0,0.5,0,1.0)));
    type = 0;
    for (i = 0; i < sphere_cnt; i++) {
        r = sdSphere(p + spheres[i].center, spheres[i].r);
        if (res >= r) {
            res = r;
            type = i + 1;
        }
    }
    for (i = 0; i < isphere_cnt; i++) {
        r = max(sdSphere(p + ispheres[i].center1, ispheres[i].r1), sdSphere(p + ispheres[i].center2, ispheres[i].r2));
        if (res >= r) {
            res = r;
            type = sphere_cnt + i + 1;
        }
    }
    for (i = 0; i < icone_cnt; i++) {
        r = max(sdCone(p+icones[i].center1, icones[i].c1), sdCone(p+icones[i].center2, icones[i].c2));
        if (res >= r) {
            res = r;
            type = sphere_cnt + isphere_cnt + i + 1;
        }
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
    } else if (type > 0 && type <= sphere_cnt) {
        //spheres
        diffColor = spheres[type-1].diffColor;
        specColor = spheres[type-1].specColor;
        specPower = spheres[type-1].specPower;
    } else if (type > sphere_cnt && type <= sphere_cnt + isphere_cnt) {
        diffColor = ispheres[type - sphere_cnt - 1].diffColor;
        specColor = ispheres[type - sphere_cnt - 1].specColor;
        specPower = ispheres[type - sphere_cnt - 1].specPower;
    } else if (type > sphere_cnt + isphere_cnt && type <= sphere_cnt + isphere_cnt + icone_cnt) {
        diffColor = icones[type - sphere_cnt - isphere_cnt - 1].diffColor;
        specColor = icones[type - sphere_cnt - isphere_cnt - 1].specColor;
        specPower = icones[type - sphere_cnt - isphere_cnt - 1].specPower;
    }

    vec3 n2   = normalize ( n );
    vec3 l2   = normalize ( l );
    vec3 v2   = normalize ( v );
    vec3 r    = reflect ( -v2, n2 );
    vec4 diff = diffColor * max ( dot ( n2, l2 ),0.9 );
    vec4 spec = specColor * pow ( max ( dot ( l2, r ), 0.7 ), specPower );

    return diff + spec;
}

float AmbientOcclusion(vec3 p, vec3 n) {
    const float ao_eps = 0.28;
    const float ao_strength = 0.1;
    float ao = 1.0, w = ao_strength/ao_eps;
    float dist = 2.0 * ao_eps;

    for (int i = 0; i < 5; i++) {
        float D = EvaluateDistance(p + n*dist);
        ao -= (dist-D) * w;
        w *= 0.5;
        dist = dist*2.0 - ao_eps;
    }
    return clamp(ao, 0.0, 1.0);
}

float SoftShadow(in vec3 ro, in vec3 rd)
{
    const float k = 4;
    float res = 1.0;
    int cnt = 0;
    float t = 0.01;
    while (cnt < 190)
    {
        float h = EvaluateDistance(ro + rd * t);
        if(h < eps)
            return 0.0;
        res = min( res, k*h/t );
        t += min(0.01,h);
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

    spheres[1].center = vec3(0.0,-0.25,0.0);
    spheres[1].r = 0.2;
    spheres[1].diffColor = vec4(0.9,0.0,0.5,1);
    spheres[1].specColor = vec4(1.0,0.07,0.57,1);
    spheres[1].specPower = 10.0;
    spheres[1].reflection = 0.7;
    spheres[1].refraction = 0.0;

    spheres[2].center = vec3(0.8,0.4,1.0);
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

    spheres[4].center = vec3(-0.4,-0.4,0.6);
    spheres[4].r = 0.2;
    spheres[4].diffColor = vec4(0.2,0.92,0.29,1);
    spheres[4].specColor = vec4(0.2,0.92,0.29,1);
    spheres[4].specPower = 10.0;
    spheres[4].reflection = 0.7;
    spheres[4].refraction = 0.0;

    spheres[5].center = vec3(0.4,-0.4,0.6);
    spheres[5].r = 0.2;
    spheres[5].diffColor = vec4(0.83,0.03,0.09,1);
    spheres[5].specColor = vec4(0.83,0.03,0.09,1);
    spheres[5].specPower = 10.0;
    spheres[5].reflection = 0.7;
    spheres[5].refraction = 0.0;

    ispheres[0].center1 = vec3(-0.5,-0.1,-0.7);
    ispheres[0].r1 = 0.3;
    ispheres[0].center2 = vec3(-0.5,-0.1,-0.3);
    ispheres[0].r2 = 0.3;
    ispheres[0].diffColor = vec4(0.1,0.7,0.9,1);
    ispheres[0].specColor = vec4(0.0,0.74,1.0,1);
    ispheres[0].specPower = 10.0;
    ispheres[0].reflection = 0.0;
    ispheres[0].refraction = 0.5;

    icones[0].center1 = vec3(-1,0.5,1.0);
    icones[0].c1 = normalize(vec2(2,-0.3));
    icones[0].center2 = vec3(-1,0.5,-1.0);
    icones[0].c2 = normalize(vec2(2,0.3));
    icones[0].diffColor = vec4(0.92,0.20,0.83,1);
    icones[0].specColor = vec4(0.90,0.2,0.8,1);
    icones[0].specPower = 10.0;
    icones[0].reflection = 0.7;
    icones[0].refraction = 0.0;
}

void InitObjects()
{
    InitSpheres();
    int i, k;
    //plane
    reflection[0] = 0.6;
    refraction[0] = 0.0;
    k = 1;
    for (i = 0; i < sphere_cnt; i++) {
        reflection[k] = spheres[i].reflection;
        refraction[k] = spheres[i].refraction;
        k++;
    }
    for (i = 0; i < isphere_cnt; i++) {
        reflection[k] = ispheres[i].reflection;
        refraction[k] = ispheres[i].refraction;
        k++;
    }
    for (i = 0; i < icone_cnt; i++) {
        reflection[k] = icones[i].reflection;
        refraction[k] = icones[i].refraction;
        k++;
    }
}

void InitLightSources()
{
    lights[0].pos = normalize(vec3(25,20,20));
    lights[0].color = vec4(0.878, 0.117, 0.741, 1);
    lights[1].pos = normalize(vec3(25,20,25));
    lights[1].color = vec4(0.231, 0.835, 0.176, 1);
    //init AA eps
    dir_eps[0] = vec3(0.0001,0.0,0.0);
    dir_eps[1] = vec3(0.0,0.0001,0.0);
    dir_eps[2] = vec3(0.0,0.0,0.0001);
}

vec4 RayMarch(vec3 ray_pos, vec3 ray_dir, inout int hit)
{
    vec3 eye_pos = ray_pos;
    int cnt = 0, ref_cnt = 0, i;
    float dt, t = 0.0;
    vec4 fc = vec4(0.0);

    hit = 0;
    while (t < tmax) {
        dt = EvaluateDistance(ray_pos);
        if (dt < 0)
            break;
        if (dt < eps) {
            hit = 1;
            //shadowing
            float sh = 0.0;
            vec4 col = vec4(0.0);
            for (i = 0; i < light_cnt; i++) {
                sh += SoftShadow(ray_pos, normalize(lights[i].pos-ray_pos));
            }
            //local coloring - Phong
            vec3 normal = EstimateNormal(ray_pos,0.0001);
            vec3 tmp = normalize(eye_pos-ray_pos);
            for (i = 0; i < light_cnt; i++) {
                col += Phong(normal,normalize(lights[i].pos - ray_pos), tmp,ray_pos);
            }
            if (refraction[type] < 0.0000001)
                col *= vec4(AmbientOcclusion(ray_pos, normal));
            col = mix(col, vec4(sh), 0.4);
            col += 0.1;
            //reflection
            if (ref_cnt >= MAX_REF_CNT)
                break;
            if (reflection[type] > 0) {
                fc = mix(fc, col, reflection[type]);
                ref_cnt++;
                vec3 tmp = normal*dot(ray_dir, normal)*(-2);
                ray_dir = normalize(tmp + ray_dir);
                ray_pos += ray_dir;
                continue;
            } else if (refraction[type] > 0) {
                fc = mix(fc,col,0.5);
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
                fc = mix(fc, col, 0.5);
                break;
            }
        }
        t+=dt;
        cnt++;
        ray_pos = ray_pos + ray_dir*dt;
    }
    if (hit == 0)
        return vec4(0);
    else
        return fc;
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

    int hit;

    fragColor = RayMarch(ray_pos, ray_dir, hit);
    for (int i = 0; i < AA_cnt; i++) {
        fragColor = mix(fragColor, RayMarch(ray_pos, ray_dir + dir_eps[i], hit), 0.5);
    }
}

