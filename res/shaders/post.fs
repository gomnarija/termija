#version 330

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;

// NOTE: Add here your custom variables

float offset = 0.0;
float frequency = 600.0/3.0;


void main()
{
    float tval = time; //time
    vec2 uv = 0.5 + (fragTexCoord - 0.5)*(0.9 + 0.01*sin(0.5*tval));
    vec4 color = texture2D(texture0, fragTexCoord);
    color = clamp(color*0.5 + 0.5*color*color*1.2, 0.0, 1.0);
    color *= 0.5 + 0.5*16.0*uv.x*uv.y*(1.0 - uv.x)*(1.0 - uv.y);
    color *= vec4(0.8, 1.0, 0.7, 1);
    color *= 0.9 + 0.1*sin(10.0*tval + uv.y*600.0);
    color *= 0.97 + 0.03*sin(110.0*tval);

    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));

    // Calculate final fragment color
    gl_FragColor = vec4(gray, gray, gray, 1.0);
}