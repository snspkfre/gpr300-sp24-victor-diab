#version 450
out vec4 FragColor; //The color of this fragment
in Surface{
	vec3 WorldPos; //Vertex position in world space
	vec3 WorldNormal; //Vertex normal in world space
	vec3 Tangent;
	vec2 TexCoord;
}fs_in;

uniform sampler2D _MainTex; 
uniform sampler2D _NormalTex; 
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;

void main(){
	vec3 N = fs_in.WorldNormal;
	vec3 T = fs_in.Tangent;
	vec3 B = cross(N, T);
	
	
	vec3 normal = texture(_NormalTex, fs_in.TexCoord).rgb;
	normal = normal * 2.0 - 1.0;

	vec3 worldNormal = normalize(
    normal.x * T +
    normal.y * B +
    normal.z * N
);

	//Light pointing straight down
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(worldNormal,toLight),0.0);
	//Calculate specularly reflected light
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	//Blinn-phong uses half angle
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(worldNormal,h),0.0),_Material.Shininess);
	//Combination of specular and diffuse reflection
	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
	lightColor+=_AmbientColor * _Material.Ka;
	vec3 objectColor = texture(_MainTex,fs_in.TexCoord).rgb;
	FragColor = vec4(objectColor * lightColor,1.0);
}