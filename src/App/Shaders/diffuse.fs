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
    mat3 ViewTBN;
    vec3 Tangent;
    vec3 Bitangent;
} fs_in;

out vec4 out_col;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D AOMap;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool blinn;
uniform int lightType;
uniform vec2 screenSize;
uniform bool useAO;
uniform bool useOnlyAO;
uniform float gAspectRatio;
uniform float gTanHalfFOV;

uniform sampler2D gDepthMap;
uniform float gSampleRad;
uniform mat4 gProj;
uniform mat4 gView;
uniform int gKernelSize;

const int MAX_KERNEL_SIZE = 128;
uniform vec3 gKernel[MAX_KERNEL_SIZE];

vec2 CalcScreenTexCoord() {
    return gl_FragCoord.xy / screenSize;
}

float CalcViewZ(vec2 Coords)
{
    float Depth = texture(gDepthMap, min(max(Coords, vec2(0.0, 0.0)), vec2(1.0, 1.0))).x;
    float ViewZ = gProj[3][2] / (1 - 2 * Depth - gProj[2][2]);
    return ViewZ;
}

float CalcAO()
{
    vec2 TexCoord = CalcScreenTexCoord();
    vec2 pos = TexCoord * 2.0 - vec2(1.0);
    vec2 ViewRay = vec2(pos.x * gAspectRatio * gTanHalfFOV, pos.y * gTanHalfFOV);

    float ViewZ = CalcViewZ(TexCoord);

    float ViewX = ViewRay.x * -ViewZ;
    float ViewY = ViewRay.y * -ViewZ;

    vec3 Pos = vec3(ViewX, ViewY, ViewZ);

    float AO = 0.0;
    vec3 tangent = fs_in.ViewTBN * normalize(fs_in.Tangent);
    vec3 normal = fs_in.ViewTBN * normalize(texture(texture_normal1, fs_in.TexCoords).rgb);
    tangent = normalize(tangent - normal * dot(tangent, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 ViewTBN = mat3(normalize(tangent), normalize(bitangent), normalize(normal));

    for (int i = 0 ; i < gKernelSize ; i++) {
        vec3 sample = ViewTBN * gKernel[i];
        if (gl_FrontFacing) {
            sample = -sample;
        }
        vec3 samplePos = Pos + sample;
        vec4 offset = vec4(samplePos, 1.0);
        offset = gProj * offset;
        offset.xy /= offset.w;
        offset.xy = (offset.xy + vec2(1.0)) / 2.0;

        float sampleDepth = CalcViewZ(offset.xy);

        if (abs(Pos.z - sampleDepth) < gSampleRad && Pos.z != sampleDepth) {
            AO += step(samplePos.z, sampleDepth);
        }
    }

    AO = 1.0 - AO/float(gKernelSize);
    return AO;
}

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
        ambient = 0.35;

        vec3 lightDir = normalize(lightPos - fs_in.FragPos);
        float diff = max(dot(lightDir, normal), 0.0);
        diffuse = diff;

        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = 0.0;
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        specular = vec3(0.2) * spec;
    }

    if (useOnlyAO) {
        float AO = pow(CalcAO(), 2.0);
        out_col = vec4(AO, AO, AO, 1.0);
    } else if (useAO) {
        float AO = pow(CalcAO(), 2.0);
        out_col = vec4(ambient * AO * color + diffuse * color + specular, 1.0);
    } else {
        out_col = vec4(ambient * color + diffuse * color + specular, 1.0);
    }
}
