uniform sampler2DShadow ShadowMap;
uniform float Epsilon;
varying vec4 ShadowCoord;
float lookup(float x, float y)
{
    float depth = shadow2DProj(ShadowMap,
                      ShadowCoord + vec3(x, y, 0) * Epsilon).x;
    return depth != 1.0 ? 0.75 : 1.0;
}
void main()
{
    float shadeFactor = lookup(0.0, 0.0);
    gl_FragColor = vec4(shadeFactor * gl_Color.rgb, gl_Color.a);
}
/*
varying vec3 DiffuseColor;
void main()
{
    gl_FragColor = vec4(DiffuseColor, 1.0);
}
*/
/*
uniform vec3  BrickColor, MortarColor;
uniform vec2  BrickSize;
uniform vec2  BrickPct;
uniform vec2  MortarPct;
varying vec2  MCposition;
varying float LightIntensity;
#define Integral(x, p, notp) ((floor(x)*(p)) + max(fract(x)-(notp), 0.0))
void main()
{
    vec2 position, fw, useBrick;
    vec3 color;
    // Determine position within the brick pattern
    position = MCposition / BrickSize;
    // Adjust every other row by an offset of half a brick
    if (fract(position.y * 0.5) > 0.5)
        position.x += 0.5;
    // Calculate filter size
    fw = fwidth(position);
    // Perform filtering by integrating the 2D pulse made by the
    // brick pattern over the filter width and height
    useBrick = (Integral(position + fw, BrickPct, MortarPct) -
                Integral(position, BrickPct, MortarPct)) / fw;
    // Determine final color
    color  = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
    color *= LightIntensity;
    gl_FragColor = vec4(color, 1.0);
}
*/
/*
uniform vec3  BrickColor, MortarColor;
uniform vec2  BrickSize;
uniform vec2  BrickPct;
varying vec2  MCposition;
varying float LightIntensity;
void main()
{
    vec3  color;
    vec2  position, useBrick;
    position = MCposition / BrickSize;
    if (fract(position.y * 0.5) > 0.5)
        position.x += 0.5;
    position = fract(position);
    useBrick = step(position, BrickPct);
    useBrick = position;
    color  = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
    color *= LightIntensity;
    gl_FragColor = vec4(color, 1.0);
}
*/
