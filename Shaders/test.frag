#version 330 core
out vec4 FragColor;

struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3  direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NUM_SPOT_LIGHT 2
#define NUM_DIR_LIGHT 2

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;
uniform DirLight dirLights[NUM_DIR_LIGHT];
uniform SpotLight spotLights[NUM_SPOT_LIGHT];
uniform Material material;

uniform bool dirLightOn; 
uniform bool lightingOn;

uniform bool lamp1On;
uniform bool lamp2On;

vec3 DirLightValue(DirLight light, vec3 normal, vec3 viewDir);
vec3 SpotLightValue(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 ExtraAmbient(SpotLight light);

void main()
{  
    vec4 texColor = texture(material.diffuse, TexCoords);
	if(texColor.a < 0.08) // smooths edges of texture and stops boxy look
        discard;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos- FragPos);
    vec3 result;

    if(lightingOn){ // used for rendering without lighting
        if (dirLightOn){ // handles directional light being on and off. 
            for(int i = 0; i < NUM_DIR_LIGHT; i++)
                result += DirLightValue(dirLights[i], norm, viewDir);
        } else {
            result += ExtraAmbient(spotLights[1]);
        }

        if (lamp1On == true){
            result += SpotLightValue(spotLights[0], norm, FragPos, viewDir);
        }
        if (lamp2On == true){
            result += SpotLightValue(spotLights[1], norm, FragPos, viewDir);
        }
        
        FragColor = vec4(result, 1.0);
    } else {
        if (!dirLightOn){ // Use Spotlights when directional lights off. 
            FragColor = vec4(0.6 * texColor.rgb, texColor.a);
        } else {
            FragColor = texColor;
        }
    }

    
}

vec3 DirLightValue(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diffuseFloat = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diffuseFloat * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    return (ambient + diffuse + specular);
}

vec3 SpotLightValue(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diffuseFloat = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diffuseFloat * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return(ambient + diffuse + specular);
}

vec3 ExtraAmbient(SpotLight light){
    return (light.ambient * vec3(texture(material.diffuse, TexCoords)));
}

