#version 450

uniform mat4 transformation;
uniform float time;
uniform float zoffset;

in vec2 pos;

//out vec3 normal;
out vec4 world;
out vec2 newpos;

const int ITER_GEOMETRY = 3;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8 * 5.0;
const float SEA_FREQ = 0.16;
const float SEA_HEIGHT = 0.6;
mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);

float hash( vec2 p ) {
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}

float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

float sea_octave(vec2 uv, float choppy) {
    uv += noise(uv);
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float map(vec2 uv) {
	float SEA_TIME = time * SEA_SPEED;

    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    uv.x *= 0.75;
    
    float d, h = 0.0;    
	{        
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
	{        
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
	{        
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return h;// p.y - h;
}

float map(float u, float v) {
    return map(vec2(u, v));
}

const float eps = 0.0001;

// via https://mobile.twitter.com/erkaman2/status/988113178537099264
vec3 calcNormal(float x, float z) {
    return normalize(vec3(map(x - eps, z) - map(x + eps, z), 2.0 * eps, map(x, z - eps) - map(x, z + eps)));
}

//const vec3 light = normalize (vec3 (2, 1, 3));

// blinn phong code from http://schorsch.efi.fh-nuernberg.de/mshopf/index.php/Computergrafik/BlinnPhongInGLSL
void main() {
	/*vec2*/ newpos = vec2(pos.x, pos.y + zoffset);
	//vec4 coord = texture2D(tex, pos);
	float height = map(newpos); //sin(pos.x + time) * sin(pos.x + time * 1.1) + sin(pos.y + time * 1.1) * sin(pos.y + time * 1.2);
    vec4 pos = vec4(newpos.x, height /*coord.r*/, newpos.y, 1.0);
	gl_Position = transformation * pos;
    world = pos;
}
