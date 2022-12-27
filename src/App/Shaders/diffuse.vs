#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform vec3 lightPos;
uniform vec3 viewPos;

out VS_OUT {
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
    mat3 ViewTBN;
    vec3 Tangent;
    vec3 Bitangent;
} vs_out;

void main() {
    vec3 modifiedPos = aPos;
    modifiedPos.y += sin(time * (modifiedPos.x + modifiedPos.z) * 2 + time * 30) * 0.4;

    vec3 posPlusTangent = aPos + aTangent * 0.01;
    posPlusTangent.y += sin(time * (posPlusTangent.x + posPlusTangent.z) * 2 + time * 30) * 0.4;

    vec3 posPlusBitangent = aPos + aBitangent * 0.01;
    posPlusBitangent.y += sin(time * (posPlusBitangent.x + posPlusBitangent.z) * 2 + time * 30) * 0.4;

    vec3 modifiedTangent = posPlusTangent - modifiedPos;
    vec3 modifiedBitangent = posPlusBitangent - modifiedPos;

    vec3 normal = normalize(cross(modifiedBitangent, modifiedTangent));

    vec3 T = normalize(vec3(model * vec4(modifiedTangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(modifiedBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal,            0.0)));

    vs_out.TBN = mat3(T, B, N);
    vs_out.Normal = normal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = vec3(model * vec4(modifiedPos, 1.0));
    vs_out.NormalMat = inverse(transpose(mat3(model)));

    normal = vs_out.NormalMat * normalize(normal);

    float ambient = 0.05;
    vs_out.Ambient = ambient;

    vec3 lightDir = normalize(lightPos - vs_out.FragPos);
    vs_out.Diffuse = max(dot(lightDir, normal), 0.0);
    vs_out.Diffuse2 = max(dot(lightDir, -normal), 0.0);

    vec3 viewDir = normalize(viewPos - vs_out.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vs_out.Specular = vec3(0.3) * spec;

    reflectDir = reflect(-lightDir, -normal);
    halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(-normal, halfwayDir), 0.0), 32.0);
    vs_out.Specular2 = vec3(0.3) * spec;

    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    vs_out.ViewTBN = mat3(normalize(normalMatrix * modifiedTangent), normalize(normalMatrix * modifiedBitangent), normalize(normalMatrix * normal));
    vs_out.Tangent = modifiedTangent;
    vs_out.Bitangent = modifiedBitangent;

	gl_Position = projection * view * model * vec4(modifiedPos, 1.0);
}
