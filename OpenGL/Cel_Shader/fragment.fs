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
const float levels = 3.0;
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
    vec3 diffuse = (lightColor * u_DiffuseStrength) * floor(diff * levels) * scaleFactor;

    // vec3 V = normalize(u_CameraPosition - v_Position);

    // Specular Lighting
    vec3 viewDirection = normalize(u_CameraPosition - v_Position);
    vec3 reflectDirection = reflect(-lightDirection, norm);
    vec3 H = normalize(lightDirection + viewDirection);

    // Cel Specular
    vec3 specular = vec3(0.0, 0.0, 0.0);
    if (dot(lightDirection, norm) > 0.0) {
        specular = u_SpecularStrength * pow(max(dot(viewDirection, norm), 0.0), 120.0);
    }
    // limit specular
    float specMask = (pow(dot(H, norm), 120.0) > 0.4) ? 1.0 : 0.0;
    // float edgeDetection = (dot(viewDirection, norm) > 0.4) ? 1.0 : 0.0;

    float spec = pow(max(dot(viewDirection, norm), 0.0), 120.0);
    // vec3 specular = u_SpecularStrength * spec * lightColor;

    vec4 sample = texture2D(u_Texture, v_UV);

    if (sample.r > u_Threshold)
    {
        discard;
    }

    // vec3 result = (ambient + diffuse + specular) * u_ObjectColor;
    // vec3 result = edgeDetection * (u_ObjectColor + diffuse + specularFloat * specMask);
    vec4 result = (vec4(u_ObjectColor, 1.0) * vec4(diffuse, 1.0) + vec4(specular, 1.0) * specMask) * sample;
    // gl_FragColor = vec4(result, 1.0);
    gl_FragColor = result;
}