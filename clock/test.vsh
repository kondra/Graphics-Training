attribute float Accessibility;
varying vec4 ShadowCoord;
// Ambient and diffuse scale factors.
const float As = 1.0 / 1.5;
const float Ds = 1.0 / 3.0;
void main()
{
    vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
    vec3 ecPosition3 = (vec3(ecPosition)) / ecPosition.w;
    vec3 VP = vec3(gl_LightSource[0].position) - ecPosition3;
    VP = normalize(VP);
    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    float diffuse = max(0.0, dot(normal, VP));
    float scale = min(1.0, Accessibility * As + diffuse * Ds);
    vec4 texCoord = gl_TextureMatrix[1] * gl_Vertex;
    ShadowCoord   = texCoord / texCoord.w;
    gl_FrontColor  = vec4(scale * gl_Color.rgb, gl_Color.a);
    gl_Position    = ftransform();
}
/*
uniform vec3 LightPosition;
uniform vec3 SkyColor;
uniform vec3 GroundColor;
void main()
{
    vec3 ecPosition = vec3(gl_ModelViewMatrix * gl_Vertex);
    vec3 tnorm = normalize(gl_NormalMatrix * gl_Normal);
    vec3 lightVec = normalize(LightPosition - ecPosition);
    float costheta = dot(tnorm, lightVec);
    float a = 0.5 + 0.5 * costheta;
    
    gl_FrontColor = mix(GroundColor, SkyColor, a);
    
    gl_Position = ftransform();
}
*/
/*
varying vec3 DiffuseColor;
uniform float ScaleFactor;
const float C1 = 0.429043;
const float C2 = 0.511664;
const float C3 = 0.743125;
const float C4 = 0.886227;
const float C5 = 0.247708;
// Constants for Old Town Square lighting
const vec3 L00  = vec3( 0.871297,  0.875222,  0.864470);
const vec3 L1m1 = vec3( 0.175058,  0.245335,  0.312891);
const vec3 L10  = vec3( 0.034675,  0.036107,  0.037362);
const vec3 L11  = vec3(-0.004629, -0.029448, -0.048028);
const vec3 L2m2 = vec3(-0.120535, -0.121160, -0.117507);
const vec3 L2m1 = vec3( 0.003242,  0.003624,  0.007511);
const vec3 L20  = vec3(-0.028667, -0.024926, -0.020998);
const vec3 L21  = vec3(-0.077539, -0.086325, -0.091591);
const vec3 L22  = vec3(-0.161784, -0.191783, -0.219152);
void main()
{
    vec3 tnorm    = normalize(gl_NormalMatrix * gl_Normal);
    
    DiffuseColor =  C1 * L22 * (tnorm.x * tnorm.x - tnorm.y * tnorm.y) +
                    C3 * L20 * tnorm.z * tnorm.z +
                    C4 * L00 -
                    C5 * L20 +
                    2.0 * C1 * L2m2 * tnorm.x * tnorm.y +
                    2.0 * C1 * L21  * tnorm.x * tnorm.z +
                    2.0 * C1 * L2m1 * tnorm.y * tnorm.z +
                    2.0 * C2 * L11  * tnorm.x +
                    2.0 * C2 * L1m1 * tnorm.y +   
                    2.0 * C2 * L10  * tnorm.z;
    
    DiffuseColor *= ScaleFactor;
    
    gl_Position = ftransform();
}
*/
/*
uniform vec3 LightPosition;
const float SpecularContribution = 0.3;
const float DiffuseContribution  = 1.0 - SpecularContribution;
varying float LightIntensity;
varying vec2  MCposition;
void main()
{
    vec3 ecPosition = vec3(gl_ModelViewMatrix * gl_Vertex);
    vec3 tnorm      = normalize(gl_NormalMatrix * gl_Normal);
    vec3 lightVec   = normalize(LightPosition - ecPosition);
    vec3 reflectVec = reflect(-lightVec, tnorm);
    vec3 viewVec    = normalize(-ecPosition);
    float diffuse   = max(dot(lightVec, tnorm), 0.0);
    float spec      = 0.0;
    if (diffuse > 0.0)
    {
        spec = max(dot(reflectVec, viewVec), 0.0);
        spec = pow(spec, 16.0);
    }
    LightIntensity = DiffuseContribution * diffuse +
                     SpecularContribution * spec;
    MCposition     = gl_Vertex.xy;
    gl_Position    = ftransform();
}
*/
