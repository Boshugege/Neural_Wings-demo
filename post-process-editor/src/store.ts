import { create } from 'zustand';
import { addEdge, applyNodeChanges, applyEdgeChanges } from 'reactflow';
import type { Connection, Edge, EdgeChange, Node, NodeChange } from 'reactflow';
import { v4 as uuidv4 } from 'uuid';
import type {
    PostProcessConfig,
    Pass,
    port,
    UniformValue,
    AllNodeData,
    PassNodeData,
    TextureNodeData
} from './types';
const generateRandomRT = () => {
    return `rt_${Math.random().toString(36).substring(2, 6)}`;
};
const generateId = () => Math.random().toString(36).substr(2, 9);
const hslToHex = (h: number, s: number, l: number) => {
    l /= 100;
    const a = s * Math.min(l, 1 - l) / 100;
    const f = (n: number) => {
        const k = (n + h / 30) % 12;
        const color = l - a * Math.max(Math.min(k - 3, 9 - k, 1), -1);
        return Math.round(255 * color).toString(16).padStart(2, '0');
    };
    return `#${f(0)}${f(8)}${f(4)}`;
};
const getTechColor = () => {
    // 色相限制在 160(青绿) 到 320(洋红) 之间，避开红橙黄
    const hue = Math.floor(Math.random() * 160) + 160;
    // 饱和度 70-90% (鲜艳)
    const sat = Math.floor(Math.random() * 20) + 70;
    // 亮度 55-65% (适中，保证文字清晰)
    const light = Math.floor(Math.random() * 10) + 55;
    return hslToHex(hue, sat, light);
};

interface State {
    nodes: Node<AllNodeData>[];
    edges: Edge[];
    onNodesChange: (changes: NodeChange[]) => void;
    onEdgesChange: (changes: EdgeChange[]) => void;
    onConnect: (connection: Connection) => void;
    updateNodeData: (nodeId: string, data: Partial<AllNodeData>) => void;
    addPassNode: () => void;
    addTextureNode: () => void;
    // 端口管理
    removeInputPort: (nodeId: string, portName: string) => void;
    renameInputPort: (nodeId: string, oldName: string, newName: string) => void;
    removeTexturePort: (nodeId: string, portName: string) => void;
    renameTexturePort: (nodeId: string, oldName: string, newName: string) => void;
    // Uniforms
    addUniform: (nodeId: string, key: string, value: UniformValue) => void;
    removeUniform: (nodeId: string, key: string) => void;
    renameUniformKey: (nodeId: string, oldKey: string, newKey: string) => void;
    updateUniformValue: (nodeId: string, key: string, value: UniformValue) => void;
    changeUniformType: (nodeId: string, key: string, type: 'float' | 'vec2' | 'vec3' | 'vec4') => void;
    updateBaseColor: (nodeId: string, color: [number, number, number, number]) => void;

    setNodeThemeColor: (nodeId: string, color: string) => void;

    generateJSON: () => PostProcessConfig;

    postProcessName: string; // 全局后处理名字
    setPostProcessName: (name: string) => void;

    importJSON: (jsonStr: string) => void;

    isRTNameDuplicate: (nodeId: string, name: string) => { error: boolean; msg?: string };
}

function getTopologicalSort(nodes: Node<AllNodeData>[], edges: Edge[]): string[] {
    const adjacencyList = new Map<string, string[]>();
    const inDegree = new Map<string, number>();

    nodes.forEach(node => {
        adjacencyList.set(node.id, []);
        inDegree.set(node.id, 0);
    });

    edges.forEach(edge => {
        if (adjacencyList.has(edge.source) && adjacencyList.has(edge.target)) {
            adjacencyList.get(edge.source)?.push(edge.target);
            inDegree.set(edge.target, (inDegree.get(edge.target) || 0) + 1);
        }
    });

    const queue: string[] = [];
    inDegree.forEach((degree, id) => {
        if (degree === 0) queue.push(id);
    });

    const result: string[] = [];
    while (queue.length > 0) {
        const u = queue.shift()!;
        result.push(u);
        adjacencyList.get(u)?.forEach(v => {
            inDegree.set(v, inDegree.get(v)! - 1);
            if (inDegree.get(v) === 0) queue.push(v);
        });
    }
    return result;
}


export const useStore = create<State>((set, get) => ({
    nodes: [
        {
            id: 'START_NODE',
            type: 'inputNode',
            position: { x: -200, y: 200 },
            data: { name: 'Input', output: 'inScreen', inputPorts: [], texturePorts: [], isStatic: true, uniforms: {} } as PassNodeData
        },
        {
            id: 'END_NODE',
            type: 'outputNode',
            position: { x: 800, y: 200 },
            data: { name: 'Output', output: 'outScreen', inputPorts: [], texturePorts: [], isStatic: true, uniforms: {} } as PassNodeData
        }
    ],
    edges: [],
    postProcessName: "后处理蓝图", // 默认名



    importJSON: (jsonStr: string) => {
        try {
            const parsed = JSON.parse(jsonStr) as PostProcessConfig;
            const data = parsed.postProcess;
            const graph = data.postProcessGraph;

            // 1. 重置画布：保留 Start/End 节点，或者完全重绘
            // 这里建议保留 Start/End 的 ID 不变，方便逻辑处理
            const startNodeId = 'START_NODE';
            const endNodeId = 'END_NODE';

            const newNodes: Node[] = [];
            const newEdges: Edge[] = [];

            // 2. 创建基础节点 (InputNode & OutputNode)
            // 这里的 ID 必须固定，或者你需要用变量记录
            newNodes.push({
                id: startNodeId,
                type: 'inputNode',
                position: { x: 50, y: 300 }, // 初始位置
                data: { label: '源: inScreen', output: 'inScreen' }
            });

            // 3. 准备映射表
            // rtMap: 记录 "RT名称" 对应的 "节点ID" (用于 RT 连线)
            const rtMap = new Map<string, string>();
            rtMap.set('inScreen', startNodeId);

            // textureMap: 记录 "图片路径" 对应的 "节点ID" (防止重复创建相同的贴图节点)
            const textureMap = new Map<string, string>();

            // 4. 遍历 JSON 生成 PassNode
            let currentX = 450; // 自动布局的起始 X 坐标
            const startY = 100;

            graph.forEach((pass, index) => {
                const nodeId = generateId(); // 为当前 Pass 生成新 ID

                // 4.1 重建 Input Ports (带稳定 ID)
                // JSON: inputs: [ ["u_tex0", "inScreen"], ... ]
                const inputPorts: port[] = [];
                const inputConnections: { portId: string, sourceRt: string }[] = [];

                pass.inputs.forEach(([portName, sourceRt]) => {
                    const portId = `in_${generateId()}`; // 生成内部连线用的 ID
                    inputPorts.push({ id: portId, name: portName });
                    // 暂存连线关系，等所有节点创建完再连
                    inputConnections.push({ portId, sourceRt });
                });

                // 4.2 重建 Texture Ports & Texture Nodes
                // JSON: textures: { "u_tex0": "path/to/img.png" }
                const texturePorts: port[] = [];
                const textureConnections: { portId: string, textureNodeId: string }[] = [];

                if (pass.textures) {
                    Object.entries(pass.textures).forEach(([portName, path]) => {
                        const portId = `tex_${generateId()}`;
                        texturePorts.push({ id: portId, name: portName });

                        // 检查该图片是否已经创建过节点，如果没有则创建
                        let texNodeId = textureMap.get(path);
                        if (!texNodeId) {
                            texNodeId = generateId();
                            textureMap.set(path, texNodeId);

                            // 创建贴图节点
                            newNodes.push({
                                id: texNodeId,
                                type: 'textureNode',
                                position: { x: currentX - 200, y: startY + 300 + (Math.random() * 100) }, // 放在 Pass 下方或左侧
                                data: {
                                    name: 'Texture',
                                    path: path,
                                    output: path,
                                    themeColor: '#3a8ee6' // 默认颜色或随机
                                }
                            });
                        }

                        // 记录连线关系
                        textureConnections.push({ portId, textureNodeId: texNodeId });
                    });
                }

                // 4.3 记录该节点的输出 RT，供后续节点查找
                if (pass.output && pass.output !== 'outScreen') {
                    rtMap.set(pass.output, nodeId);
                }

                // 如果直接输出到屏幕，记录一下，稍后连线到 END_NODE
                const isOutputToScreen = pass.output === 'outScreen';

                // 4.4 创建 PassNode 对象
                newNodes.push({
                    id: nodeId,
                    type: 'passNode',
                    position: { x: currentX, y: startY + (index % 2) * 50 }, // 简单的错位布局
                    data: {
                        name: pass.name,
                        vs: pass.vs,
                        fs: pass.fs,
                        inputPorts: inputPorts,
                        texturePorts: texturePorts,
                        uniforms: pass.uniforms || {},
                        output: pass.output,
                        baseColor: pass.baseColor,
                        // 使用之前的随机颜色生成逻辑，或者从 JSON 读取(如果 JSON 有存的话)
                        themeColor: getTechColor()
                    }
                });

                // 5. 生成连线 (Edges)

                // 5.1 输入端口连线
                inputConnections.forEach(({ portId, sourceRt }) => {
                    const sourceNodeId = rtMap.get(sourceRt);
                    if (sourceNodeId) {
                        newEdges.push({
                            id: `edge_${sourceNodeId}_${nodeId}_${portId}`,
                            source: sourceNodeId,
                            sourceHandle: 'output', // 假设所有源节点的输出 Handle ID 都是 'output'
                            target: nodeId,
                            targetHandle: portId, // 关键：连接到刚才生成的稳定 ID
                            animated: true,
                            style: { stroke: '#999' }
                        });
                    }
                });

                // 5.2 贴图端口连线
                textureConnections.forEach(({ portId, textureNodeId }) => {
                    newEdges.push({
                        id: `edge_${textureNodeId}_${nodeId}_${portId}`,
                        source: textureNodeId,
                        sourceHandle: 'output',
                        target: nodeId,
                        targetHandle: portId,
                        style: { stroke: '#3a8ee6' } // 蓝色线
                    });
                });

                // 5.3 如果是最终输出，连接到 EndNode
                if (isOutputToScreen) {
                    newEdges.push({
                        id: `edge_${nodeId}_${endNodeId}`,
                        source: nodeId,
                        sourceHandle: 'output',
                        target: endNodeId,
                        targetHandle: 'input', // 假设 EndNode 的输入 Handle ID 是 input
                        animated: true,
                        style: { stroke: '#e84855', strokeWidth: 2 }
                    });
                }

                // 布局步进
                currentX += 450;
            });

            // 6. 添加 OutputNode (放在最后)
            newNodes.push({
                id: endNodeId,
                type: 'outputNode',
                position: { x: currentX, y: 300 },
                data: { label: '输出: outScreen' }
            });

            // 7. 更新 Store
            set({
                nodes: newNodes,
                edges: newEdges,
                postProcessName: data.name || 'Imported Graph'
            });

        } catch (e) {
            console.error("Import JSON failed:", e);
            alert("导入失败，JSON 格式错误");
        }
    },


    setPostProcessName: (name: string) => set({ postProcessName: name }),
    setNodeThemeColor: (nodeId, color) => {
        const { nodes } = get();
        set({
            nodes: nodes.map(n => n.id === nodeId ? {
                ...n,
                data: { ...n.data, themeColor: color }
            } : n)
        });
    },
    onNodesChange: (changes) => {
        const filteredChanges = changes.filter(c => {
            if (c.type === 'remove') {
                const node = get().nodes.find(n => n.id === c.id);
                // 此时 node.data 类型已知，可以安全访问 isStatic
                return !(node?.data as PassNodeData).isStatic;
            }
            return true;
        });
        set({ nodes: applyNodeChanges(filteredChanges, get().nodes) });
    },

    onEdgesChange: (changes) => set({ edges: applyEdgeChanges(changes, get().edges) }),
    onConnect: (connection: Connection) => {
        const { edges } = get();

        const filteredEdges = edges.filter(
            (edge) => !(edge.target === connection.target && edge.targetHandle === connection.targetHandle)
        );

        set({
            edges: addEdge(connection, filteredEdges)
        });
    },
    isRTNameDuplicate: (nodeId: string, name: string) => {
        if (name === 'inScreen') return { error: true, msg: 'inScreen 是保留输入名，不可作为输出' };
        const nodes = get().nodes;
        const exists = nodes.some(n =>
            n.id !== nodeId &&
            n.type === 'passNode' &&
            (n.data as PassNodeData).output === name
        );

        return exists ? { error: true, msg: `RT名称 "${name}" 已存在，请使用唯一名称` } : { error: false };
    },

    updateNodeData: (nodeId, newData) => set({

        nodes: get().nodes.map(n => n.id === nodeId ? { ...n, data: { ...n.data, ...newData } } : n)
    }),

    removeTexturePort: (nodeId, portId) => {
        const { nodes, edges } = get();
        set({
            nodes: nodes.map(n => {
                if (n.id !== nodeId) return n;
                const d = n.data as PassNodeData;
                return {
                    ...n,
                    data: {
                        ...d,
                        texturePorts: d.texturePorts.filter(p => p.id !== portId)
                    }
                };
            }),
            edges: edges.filter(e => !(e.target === nodeId && e.targetHandle === portId))
        });
    },
    renameTexturePort: (nodeId, portId, newName) => {
        set((state) => ({
            nodes: state.nodes.map((node) => {
                if (node.id === nodeId) {
                    const data = node.data as PassNodeData;
                    return {
                        ...node,
                        data: {
                            ...data,
                            texturePorts: data.texturePorts.map((p) =>
                                p.id === portId ? { ...p, name: newName } : p
                            ),
                        },
                    };
                }
                return node;
            }),
        }));
    },

    removeInputPort: (nodeId, portId) => {
        const { nodes, edges } = get();
        set({
            nodes: nodes.map(n => {
                if (n.id !== nodeId) return n;
                const d = n.data as PassNodeData;
                return {
                    ...n,
                    data: {
                        ...d,
                        inputPorts: d.inputPorts.filter(p => p.id !== portId)
                    }
                };
            }),
            edges: edges.filter(e => !(e.target === nodeId && e.targetHandle === portId))
        });
    },

    renameInputPort: (nodeId, portId, newName) => {
        set((state) => ({
            nodes: state.nodes.map((node) => {
                if (node.id === nodeId) {
                    const data = node.data as PassNodeData;
                    return {
                        ...node,
                        data: {
                            ...data,
                            inputPorts: data.inputPorts.map((p) =>
                                p.id === portId ? { ...p, name: newName } : p
                            ),
                        },
                    };
                }
                return node;
            }),
        }));
    },

    addUniform: (nodeId, key, value) => set({
        nodes: get().nodes.map(n => {
            if (n.id !== nodeId) return n;
            const d = n.data as PassNodeData;
            return { ...n, data: { ...d, uniforms: { ...d.uniforms, [key]: value } } };
        })
    }),

    removeUniform: (nodeId, key) => set({
        nodes: get().nodes.map(n => {
            if (n.id !== nodeId) return n;
            const d = n.data as PassNodeData;
            const next = { ...d.uniforms }; delete next[key];
            return { ...n, data: { ...d, uniforms: next } };
        })
    }),

    renameUniformKey: (nodeId, oldKey, newKey) => set({
        nodes: get().nodes.map(n => {
            if (n.id !== nodeId || oldKey === newKey) return n;
            const d = n.data as PassNodeData;
            const next = { ...d.uniforms };
            const val = next[oldKey];
            delete next[oldKey]; next[newKey] = val;
            return { ...n, data: { ...d, uniforms: next } };
        })
    }),

    updateUniformValue: (nodeId, key, value) => set({
        nodes: get().nodes.map(n => {
            if (n.id !== nodeId) return n;
            const d = n.data as PassNodeData;
            return { ...n, data: { ...d, uniforms: { ...d.uniforms, [key]: value } } };
        })
    }),

    changeUniformType: (nodeId, key, type) => {
        const map: Record<string, UniformValue> = { float: 0, vec2: [0, 0], vec3: [0, 0, 0], vec4: [0, 0, 0, 0] };
        set({
            nodes: get().nodes.map(n => {
                if (n.id !== nodeId) return n;
                const d = n.data as PassNodeData;
                return { ...n, data: { ...d, uniforms: { ...d.uniforms, [key]: map[type] } } };
            })
        });
    },

    updateBaseColor: (nodeId, color) => set({
        nodes: get().nodes.map(n => {
            if (n.id !== nodeId) return n;
            const d = n.data as PassNodeData;
            return { ...n, data: { ...d, baseColor: color } };
        })
    }),

    addTextureNode: () => {
        const position = {
            x: Math.random() * 400 + 100,
            y: Math.random() * 300 + 100
        };
        set({
            nodes: [...get().nodes, {
                id: uuidv4(),
                type: 'textureNode',
                position: position,
                data: { name: 'TextureAsset', path: 'assets/textures/test.png', output: 'assets/textures/test.png', themeColor: getTechColor() } as TextureNodeData,

            }]
        })
    },

    addPassNode: () => {
        const { nodes } = get();
        let newRTName = generateRandomRT();
        while (nodes.some(n => n.type === 'passNode' && (n.data as PassNodeData).output === newRTName)) {
            newRTName = generateRandomRT();
        }
        const position = {
            x: Math.random() * 400 + 100,
            y: Math.random() * 300 + 100
        };
        const newNode = {
            id: generateId(),
            type: 'passNode',
            position,
            data: {
                name: 'New Pass',
                inputPorts: [{ id: `in_${generateId()}`, name: 'u_tex0' }],
                texturePorts: [],
                uniforms: {},
                output: newRTName,
                themeColor: getTechColor()
            }
        };
        set({ nodes: [...get().nodes, newNode] });
    },

    generateJSON: () => {
        const { nodes, edges, postProcessName } = get();
        const sortedIds = getTopologicalSort(nodes, edges);
        const sortedPasses: Pass[] = [];
        const rtPoolSet = new Set<string>(['inScreen', 'outScreen']);

        sortedIds.forEach(id => {
            const node = nodes.find(n => n.id === id);
            if (!node || node.type !== 'passNode') return;
            const data = node.data as PassNodeData;

            const inputs = edges
                .filter(e => e.target === id)
                .map(e => {
                    const port = (data.inputPorts || []).find(p => p.id === e.targetHandle);
                    if (!port) return null;

                    const sourceNode = nodes.find(n => n.id === e.source);
                    if (!sourceNode) return [port.name, ''] as [string, string];

                    const output = sourceNode.type === 'textureNode'
                        ? (sourceNode.data as TextureNodeData).output
                        : (sourceNode.data as PassNodeData).output;

                    return [port.name || 'u_texture', output] as [string, string];
                }).filter((item): item is [string, string] => item !== null);

            const textures: Record<string, string> = {};
            edges
                .filter(e => e.target === id)
                .forEach(e => {
                    const port = (data.texturePorts || []).find(p => p.id === e.targetHandle);
                    if (!port) return;

                    const src = nodes.find(n => n.id === e.source)!;
                    if (src && src.type === 'textureNode') textures[port.name] = (src.data as TextureNodeData).path;
                });

            const connectsToOutput = edges.some(e => e.source === id && e.target === 'END_NODE');
            const finalOutputName = connectsToOutput ? 'outScreen' : data.output;
            if (finalOutputName !== 'outScreen' && finalOutputName !== 'inScreen') rtPoolSet.add(finalOutputName);

            sortedPasses.push({
                name: data.name,
                vs: data.vs || 'assets/shaders/postprocess/default.vs',
                fs: data.fs || 'assets/shaders/postprocess/default.fs',
                inputs: inputs.length > 0 ? inputs : [["u_texture", "inScreen"]],
                output: finalOutputName,
                uniforms: data.uniforms,
                textures: Object.keys(textures).length > 0 ? textures : undefined,
                baseColor: data.baseColor
            });
        });

        return { postProcess: { name: postProcessName, rtPool: Array.from(rtPoolSet), hint: "拓扑序", postProcessGraph: sortedPasses } };
    }
}));