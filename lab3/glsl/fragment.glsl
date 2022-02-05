varying vec3 normal;
varying vec4 world_position;

void main()
{
    vec4 ambient = vec4(1, 0, 0, 1);
    vec4 diffuse = vec4(0, 1, 0, 1);
    vec4 specular = vec4(0, 0, 1, 1);

    ambient = gl_LightSource[0].ambient * gl_LightModel.ambient * gl_FrontMaterial.ambient;
    
    vec3 light = gl_LightSource[0].position.xyz - world_position.xyz;
    diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(light, normal), 0.0);

    vec3 r = 2.0 * dot(normal, light) * normal - light;
    vec4 view = vec4(0,0,0,0) - world_position;
    vec4 reflected = vec4(r[0], r[1], r[2], 0);
    specular = gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(view, reflected), 0.0), gl_FrontMaterial.shininess);
    
    gl_FragColor = ambient + diffuse + specular;
}
