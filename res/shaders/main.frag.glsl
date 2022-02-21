#version 460 core

in VS_OUT {
	vec2 UVs;
	flat mat3 TBN;
	vec3 fragPos;
	vec4 FragPosLightSpace; // for shadow Mapping
} fs_in;

// Material:
struct Material {
	sampler2D diffuseMap;
	vec3 diffuseColor;

	sampler2D specularMap;
    vec3 specularColor;

    float shininess;
};
uniform Material material;

// PointLight
struct PointLight {
    vec3 position;

	float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];

// Directional Light:
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform DirLight dirLight;




uniform vec3 viewPos;
uniform sampler2D normalMap;

// uniform sampler2D shadowMap;
uniform sampler2DShadow shadowMap;

out vec4 FragColor;


vec3 choose(sampler2D tex, vec3 value) {
	if(textureSize(tex, 0).x != 1)
	// if(textureQueryLevels(tex) != 0)
		return texture(tex, fs_in.UVs).rgb;
	return value;
}

// Material properties for 1 specific Fragment:
struct MaterialPoint {
    vec3 diffuse;
    vec3 specular;
    float shininess;
};


vec3 CalcDirLight(MaterialPoint material, DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(MaterialPoint material, PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


float ShadowCalculation() {
    // perform perspective divide
    // vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    vec3 projCoords = vec3(fs_in.FragPosLightSpace.xy, fs_in.FragPosLightSpace.z / fs_in.FragPosLightSpace.w-.005*0.);
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    // float closestDepth = texture(shadowMap, projCoords.xy).r;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;



	// float bias = .0003;
	vec3 lightDir = -dirLight.direction; // direction from texel to light
	vec3 normal = normalize(fs_in.TBN * vec3(0., 0., 1.)); // TODO change: (recalculating normal unneccessarily)
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.001);

	if(projCoords.z > 1.0)
        return 0.0;

    // return 1. - texture(shadowMap, projCoords.xyz, bias);
    // return 1. - texture(shadowMap, projCoords.xyz-vec3(0., 0., bias));
    // return currentDepth - bias > closestDepth  ? 1.0 : 0.0; // original

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x) {
		for(int y = -1; y <= 1; ++y) {
			// float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			// shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
			shadow += 1. - texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, projCoords.z-bias));
		}
	}
	shadow /= 9.0;
    return shadow;

}


void main() {
	// vec3 norm = normalize(normal);
	// vec3 norm = texture(normalMap, UVs).rgb;
	// vec3 norm = texture(normalMap, UVs).rgb * 2. - vec3(1.);

	MaterialPoint mp;
	mp.diffuse = choose(material.diffuseMap, material.diffuseColor);
	mp.specular = choose(material.specularMap, material.specularColor);
	// mp.specular = material.specularColor;
	mp.shininess = 32.;//material.shininess;

	vec3 norm = texture(normalMap, fs_in.UVs).rgb * 2.0 - 1.0;
	// vec3 norm = vec3(0., 0., 1.);
	norm = normalize(fs_in.TBN * norm);

	vec3 viewDir = normalize(viewPos - fs_in.fragPos);

    // phase 1: Directional lighting
    vec3 result = CalcDirLight(mp, dirLight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(mp, pointLights[i], norm, fs_in.fragPos, viewDir);
    // phase 3: Spot light
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
}



vec3 CalcDirLight(MaterialPoint material, DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // combine results
    // vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, fs_in.UVs));
    vec3 ambient  = light.ambient * material.diffuse;

    // vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, fs_in.UVs));
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;

    // vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.UVs)); // specular texture
    vec3 specular = light.specular * spec * material.specular; // specular value

	diffuse *= (1. - ShadowCalculation());
	specular *= (1. - ShadowCalculation());
	
    return (ambient + diffuse + specular);
}


vec3 CalcPointLight(MaterialPoint material, PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // Blinn Shading
	vec3 halfwayDir = normalize(lightDir + viewDir); // Blinn-Phong Shading
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess); // Blinn-Phong Shading

    // combine results
    // vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, fs_in.UVs));
    vec3 ambient  = light.ambient  * material.diffuse;

    // vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, fs_in.UVs));
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;

    // vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.UVs)); // specular texture
    vec3 specular = light.specular * spec * material.specular; // specular value

    // attenuation
    float distance    = length(light.position - fragPos);
    // float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)*0);

    return (ambient + diffuse + specular) * attenuation;
}