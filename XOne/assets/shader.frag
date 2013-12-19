#version 110

uniform sampler2D tex0;
varying vec3 normal;
varying vec3 worldPosition;
varying vec3 viewDir;
varying vec3 position;

void main()
{
	vec3 sample = texture2D( tex0, gl_TexCoord[0].st ).rgb;

	vec3 ppNormal = normalize( normal );
	vec3 L = normalize( vec3( 0.0, 0.0, 1.0 ) );
	vec3 eye = vec3( 0.0, 0.0, 1.0 );	
	vec3 R = ppNormal * 2.0 * ( dot( L, ppNormal ) ) - L;
	vec3 V = normalize( viewDir );
	

	//float ppDiffuse1 = max( dot( V, ppNormal ), 0.0 );
	float ppDiffuse = max( dot( eye, ppNormal ), 0.0 );
	float ppSpecular = pow( ppDiffuse, 32.0 ) * 1.0;
	//float filtre = max( dot( worldPosition, ppNormal ), 0.0 );

	//if(filtre > 0.5 ) gl_FragColor.rgb = sample * ppDiffuse;// + vec3( ppSpecular, ppSpecular, ppSpecular );
	//else gl_FragColor.rgb = vec3( 1.0, 1.0, 1.0 )* ppDiffuse;
	gl_FragColor.rgb = sample * ppDiffuse;
	//gl_FragColor.rgb = sample * pow(ppDiffuse-0.4 ,2.0) * 5.0;
	gl_FragColor.a = 1.0;
}
