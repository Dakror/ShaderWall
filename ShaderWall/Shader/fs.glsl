out vec4 frag_color;
in vec2 coords;
uniform vec3 iResolution;
uniform float iGlobalTime;
uniform vec4 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;
uniform sampler2D whiteScreen;
uniform sampler2D blackScreen;
uniform vec3 iChannelResolution[4];
uniform int monitorCount;
uniform ivec4 taskBars[]; // solve the issue that we need a buffer here
/*shader*/

void main() 
{
	vec2 fullCoords = coords*iResolution.rg;

    vec4 color = /*main_image*/(fullCoords);
    vec3 white = texture2D(whiteScreen, coords).rgb;
    vec3 black = texture2D(blackScreen, coords).rgb;
	bool isOk = true;
    if(all(greaterThan(white, vec3(0.85))) && all(lessThan(black, vec3(0.15)))) isOk = false;

	for (int i = 0; i < monitorCount; i++) {
		if(all(greaterThanEqual(fullCoords, taskBars[i].xy)) && all(lessThanEqual(fullCoords, vec2(taskBars[i].x + taskBars[i].z, taskBars[i].y + taskBars[i].w)))) {
			isOk = false;
			break;
		}
	}

    frag_color = isOk ? color : vec4(black, 1.0);
}





