varying vec3 normal;
varying vec4 world_position;

void main()
{
    vec4 ambient = vec4(1, 0, 0, 1);
    vec4 diffuse = vec4(0, 1, 0, 1);
    vec4 specular = vec4(0, 0, 1, 1);
    
    

    ambient = gl_FrontMaterial.ambient + gl_LightSource[0].ambient * gl_LightModel.ambient;
    
    vec3 light = normalize(gl_LightSource[0].position.xyz - world_position.xyz);
    diffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * max(dot(light, normal), 0.0);

    vec3 r = normalize(2.0 * dot(normal, light) * normal - light);
    vec4 view =  -world_position;
    specular =  gl_LightSource[0].specular * gl_FrontMaterial.specular * pow(max(dot(normalize(view.xyz), r), 0.0), gl_FrontMaterial.shininess);
    
    gl_FragColor = ambient + diffuse + specular;
}
