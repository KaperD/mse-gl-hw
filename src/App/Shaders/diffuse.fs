#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 NormalMat;
    mat3 TBN;
    float Ambient;
    float Diffuse;
    vec3 Specular;
    float Diffuse2;
    vec3 Specular2;
} fs_in;

out vec4 out_col;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool blinn;
uniform int lightType;

void main() {
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;

    float ambient;
    float diffuse;
    vec3 specular;

    if (lightType == 1) { // vertex
        ambient = fs_in.Ambient;
        if (!gl_FrontFacing) {
            diffuse = fs_in.Diffuse;
            specular = fs_in.Specular;
        } else {
            diffuse = fs_in.Diffuse2;
            specular = fs_in.Specular2;
        }
    } else {
        vec3 normal;
        if (lightType == 2) { // fragment
            normal = fs_in.NormalMat * normalize(fs_in.Normal);
        } else { // map
            normal = fs_in.NormalMat * texture(texture_normal1, fs_in.TexCoords).rgb;
            normal = normalize(normal * 2.0 - 1.0);
            normal = normalize(fs_in.TBN * normal);
        }
        if (gl_FrontFacing) {
            normal = -normal;
        }
        ambient = 0.05;

        vec3 lightDir = normalize(lightPos - fs_in.FragPos);
        float diff = max(dot(lightDir, normal), 0.0);
        diffuse = diff;

        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = 0.0;
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        specular = vec3(0.3) * spec;
    }

	out_col = vec4(ambient * color + diffuse * color + specular, 1.0);
}
