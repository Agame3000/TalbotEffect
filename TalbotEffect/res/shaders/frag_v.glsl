uniform vec2 iRes;

const int MAX_K = 1000;

uniform vec2 fieldSize;
uniform vec2 offset;
uniform float d;
uniform int type;
uniform float c;
uniform int k;
uniform float zT;
uniform vec3 color;
uniform vec2 a0;
uniform vec2 an[MAX_K * 2];

const float PI = 3.141592653589793;

void main() {
    vec2 uv = gl_FragCoord.xy / iRes;
    uv *= fieldSize;
    uv += offset;

    vec2 sc = vec2(0.0, 0.0);
    vec2 ss = vec2(0.0, 0.0);
    for(int n = -k; n < k; n++) {
        if (n == 0) continue;
        float fn = float(n);
        float t = 2.0*PI*(fn*uv.y/d-fn*fn*uv.x/zT);
        sc += an[n+k]*cos(t);
        ss += an[n+k]*sin(t);
    }

    vec2 fsc = a0 + sc;
    float I = 0.0;
    if (type == 0) I = fsc.x*fsc.x - fsc.y*fsc.y + ss.x*ss.x - ss.y*ss.y;
    else if (type == 1) I = 2.0*fsc.x*fsc.y + 2.0*ss.x*ss.y;

    gl_FragColor = vec4(color * I * c, 1.0);
}