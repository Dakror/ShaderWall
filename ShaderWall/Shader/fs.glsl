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
uniform ivec4 taskBars[/*taskbars*/];
/*shader*/

void main() 
{
    vec2 fullCoords = coords*iResolution.rg;
    vec4 color = /*main_image*/(fullCoords);
    vec3 white = texture2D(whiteScreen, coords).rgb;
    vec3 black = texture2D(blackScreen, coords).rgb;
    int mode = 0;
    if(!(all(greaterThan(white, vec3(0.85))) && all(lessThan(black, vec3(0.15))))) mode = 1;

    for (int i = 0; i < taskBars.length(); i++) {
        if(all(greaterThanEqual(fullCoords, taskBars[i].xw)) && all(lessThanEqual(fullCoords, taskBars[i].zy))) {
            mode = 2;
            break;
        }
	}
    frag_color = mode == 0 ? vec4(color.rgb, 1.0) : (mode == 1 ? vec4(black, 1.0) : vec4(0.0));
    /*int i = 0;
    if(fullCoords.x >= taskBars[i].x && fullCoords.y <= taskBars[i].y && fullCoords.x <= taskBars[i].z && fullCoords.y >= taskBars[i].w) 
	frag_color = vec4(1.0,0.0,0.0,1.0);*/
}
