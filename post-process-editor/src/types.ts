export type UniformValue = number | number[];
export type UniformType = 'float' | 'vec2' | 'vec3' | 'vec4';


export interface port {
    id: string;
    name: string;
}
export interface Pass {
    name: string;
    vs: string;
    fs: string;
    inputs: [string, string][];
    output: string;
    uniforms: { [key: string]: UniformValue };
    baseColor?: [number, number, number, number];
    textures?: { [key: string]: string };
}

export interface PostProcessConfig {
    postProcess: {
        name: string;
        rtPool: string[];
        hint: string;
        postProcessGraph: Pass[];
    };
}

export interface PassNodeData {
    name: string;
    vs?: string;
    fs?: string;

    inputPorts: port[];
    texturePorts: port[];
    output: string;
    uniforms: { [key: string]: UniformValue };
    isStatic?: boolean;
    baseColor?: [number, number, number, number];
    themeColor?: string;
}

export interface TextureNodeData {
    name: string;
    path: string;
    output: string;
    themeColor?: string;
}

export type AllNodeData = PassNodeData | TextureNodeData;