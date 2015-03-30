// vertex shader

#version 150

// these are for the programmable pipeline system and are passed in
// by default from OpenFrameworks
uniform mat4 modelViewProjectionMatrix;

in vec4 position;     //  single vertex pos
in vec2 texcoord;

// this is something we're creating for this shader
out vec2 texCoordVarying;


void main(){
    
    // here we move the texture coordinates
    texCoordVarying = vec2(texcoord.x, texcoord.y);
    
     /* what comes out of the vertex shader
        fixed role: pass the position of vertex to "triangle assembly"
        "triangle assembly": when GPU connects vertives to form triangles
    */
    gl_Position = modelViewProjectionMatrix * position;  // send the vertices to the fragment shader
}