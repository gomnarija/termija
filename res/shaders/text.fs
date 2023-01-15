//https://www.shadertoy.com/view/lsXGWn
#version 330

precision mediump float;

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

const float blurSize = 1.5/512.0;
const float intensity = 2.3;

void main(){

   vec4 sum = vec4(0);
   int j;
   int i;

   // blur in y (vertical)
   // take nine samples, with the distance blurSize between them
   sum += texture(texture0, vec2(fragTexCoord.x - 4.0*blurSize, fragTexCoord.y)) * 0.05;
   sum += texture(texture0, vec2(fragTexCoord.x - 3.0*blurSize, fragTexCoord.y)) * 0.09;
   sum += texture(texture0, vec2(fragTexCoord.x - 2.0*blurSize, fragTexCoord.y)) * 0.12;
   sum += texture(texture0, vec2(fragTexCoord.x - blurSize, fragTexCoord.y)) * 0.15;
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y)) * 0.16;
   sum += texture(texture0, vec2(fragTexCoord.x + blurSize, fragTexCoord.y)) * 0.15;
   sum += texture(texture0, vec2(fragTexCoord.x + 2.0*blurSize, fragTexCoord.y)) * 0.12;
   sum += texture(texture0, vec2(fragTexCoord.x + 3.0*blurSize, fragTexCoord.y)) * 0.09;
   sum += texture(texture0, vec2(fragTexCoord.x + 4.0*blurSize, fragTexCoord.y)) * 0.05;
	
	// blur in y (vertical)
   // take nine samples, with the distance blurSize between them
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y - 4.0*blurSize)) * 0.05;
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y - 3.0*blurSize)) * 0.09;
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y - 2.0*blurSize)) * 0.12;
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y - blurSize)) * 0.15;
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y)) * 0.16;
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y + blurSize)) * 0.15;
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y + 2.0*blurSize)) * 0.12;
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y + 3.0*blurSize)) * 0.09;
   sum += texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y + 4.0*blurSize)) * 0.05;

   gl_FragColor = sum*1.023 + texture(texture0, fragTexCoord);

}