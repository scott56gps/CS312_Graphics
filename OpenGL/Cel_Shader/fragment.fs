uniform sampler2D u_Texture;
uniform float u_Threshold;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 u_ObjectColor;
uniform vec3 u_CameraPosition;
uniform vec3 u_AmbientStrength;
uniform vec3 u_DiffuseStrength;
uniform vec3 u_SpecularStrength;

varying vec2 v_UV;
varying vec3 v_Normal;
varying vec3 v_Position;

// Cel Shader
const float levels = 5.0;
const float scaleFactor = 1.0 / levels;
const vec3 diffuseColor = vec3(0.30,0.80,0.10);

void main()
{
    // Ambient Lighting
    vec3 ambient = u_AmbientStrength * lightColor;

    // Diffuse Lighting
    vec3 norm = normalize(v_Normal);
    vec3 lightDirection = normalize(lightPosition - v_Position);
    float diff = max(dot(norm, lightDirection), 0.0);
    // vec3 diffuse = diff * (lightColor * u_DiffuseStrength);

    // Cel Diffuse
    vec3 diffuse = (u_ObjectColor * u_DiffuseStrength) * floor(diff * levels) * scaleFactor;

    vec3 V = normalize(u_CameraPosition - v_Position);
    vec3 H = normalize(lightDirection + V);

    // Specular Lighting
    vec3 viewDirection = normalize(u_CameraPosition - v_Position);
    vec3 reflectDirection = reflect(-lightDirection, norm);

    // Cel Specular
    float specularFloat = 0.0;
    if (dot(lightDirection, v_Normal) > 0.0) {
        specularFloat = pow(max(dot(viewDirection, reflectDirection), 0.0), 32.0);
    }
    // limit specular
    float specMask = (pow(dot(H, reflectDirection), 32.0) > 0.01) ? 1.0 : 0.0;
    float edgeDetection = (dot(V, reflectDirection) > 0.01) ? 1.0 : 0.0;

    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32.0);
    // vec3 specular = u_SpecularStrength * spec * lightColor;

    // vec4 sample = texture2D(u_Texture, v_UV);

    // if (sample.r > u_Threshold)
    // {
    //     discard;
    // }

    // vec3 result = (ambient + diffuse + specular) * u_ObjectColor;
    vec3 result = edgeDetection * (u_ObjectColor + diffuse + specularFloat * specMask);
    gl_FragColor = vec4(result, 1.0);
}