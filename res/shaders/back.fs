#version 330
precision mediump float;

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;


vec2 brownConradyDistortion(in vec2 uv, in float k1, in float k2)
{
    uv = uv * 2.0 - 1.0;	// brown conrady takes [-1:1]

    // positive values of K1 give barrel distortion, negative give pincushion
    float r2 = uv.x*uv.x + uv.y*uv.y;
    uv *= 1.0 + k1 * r2 + k2 * r2 * r2;
    
    // tangential distortion (due to off center lens elements)
    // is not modeled in this function, but if it was, the terms would go here
    
    uv = (uv * .5 + .5);	// restore -> [0:1]
    return uv;
}

void main(){
    vec2 resolution = vec2(800.0, 600.0);
    vec2 q = fragTexCoord.xy;
    vec2 uv = q;
    
    vec3 col = texture(texture0,uv).rgb;
    
    float k1 = 0.032;
    float k2 = 0.0;
    uv = brownConradyDistortion( uv, k1, k2);

    // darken outside uv range
    vec2 uv2 = abs(uv * 2. - 1.);
    vec2 border = 1.-smoothstep(vec2(.95),vec2(1.0),uv2);
    col *= mix(.0, 1.0, border.x * border.y);

    // vignette
    float vignetteRange = clamp(k1, 0., 1.5);
    float dist = distance(uv, vec2(0.5, 0.5));
    dist = (dist - (.707 - vignetteRange)) / vignetteRange;
    float mult = smoothstep(1.0, .0, dist);
    col *= mult;

	if (uv.x < 0.0 || uv.x > 1.0)
		col *= 0.0;
	if (uv.y < 0.0 || uv.y > 1.0)
		col *= 0.0;

    // Calculate final fragment color
    gl_FragColor = vec4(col, 1.0);

}