import { create } from 'zustand';
// @ts-expect-error - bala
import { addEdge, applyNodeChanges, applyEdgeChanges, MarkerType } from 'reactflow';
// @ts-expect-error - bala
import type { Connection, Edge, EdgeChange, Node, NodeChange } from 'reactflow';
import { v4 as uuidv4 } from 'uuid';
import type {
    PostProcessConfig,
    Pass,
    port,
    UniformValue,
    AllNodeData,
    PassNodeData,
    TextureNodeData,
    ParticleNodeData
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
    addParticleNode: () => void;
    setNodeThemeColor: (nodeId: string, color: string) => void;

    generateJSON: () => PostProcessConfig;

    postProcessName: string; // 全局后处理名字
    setPostProcessName: (name: string) => void;

    importJSON: (jsonStr: string) => void;

    isRTNameDuplicate: (nodeId: string, name: string) => { error: boolean; msg?: string };
    showDepthLines: boolean;
    toggleDepthLines: () => void;
    disconnectHandle: (nodeId: string, handleId: string) => void;
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
    showDepthLines: true, // 默认为显示
    toggleDepthLines: () => set({ showDepthLines: !get().showDepthLines }),
    disconnectHandle: (nodeId: string, handleId: string) => {
        const { edges } = get();
        const newEdges = edges.filter(edge =>
            // 过滤掉 连接到该节点 且 连接到该插槽 的线
            !((edge.target === nodeId && edge.targetHandle === handleId) ||
                (edge.source === nodeId && edge.sourceHandle === handleId))
        );

        // 只有当数量发生变化时才更新，避免不必要的重渲染
        if (newEdges.length !== edges.length) {
            set({ edges: newEdges });
        }
    },
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
    postProcessName: "帧合成蓝图", // 默认名

    addParticleNode: () => {
        const id = uuidv4();
        set({
            nodes: [
                ...get().nodes,
                {
                    id,
                    type: 'particleNode',
                    position: { x: 100, y: 500 },
                    data: {
                        name: 'Particle',
                        output: generateRandomRT(),
                        themeColor: getTechColor(), // 粒子默认橙色
                    } as ParticleNodeData,
                },
            ],
        });
    },
    importJSON: (jsonStr: string) => {
        try {
            const parsed = JSON.parse(jsonStr) as PostProcessConfig;
            const data = parsed.postProcess;
            const graph = data.postProcessGraph;

            // 1. 重置画布
            const startNodeId = 'START_NODE';
            const endNodeId = 'END_NODE';

            const newNodes: Node<AllNodeData>[] = []; // 使用泛型确保类型安全
            const newEdges: Edge[] = [];

            // 2. 创建基础节点
            newNodes.push({
                id: startNodeId,
                type: 'inputNode',
                position: { x: 50, y: 300 },
                data: { name: 'Input', output: 'inScreen', inputPorts: [], texturePorts: [], isStatic: true, uniforms: {} } as PassNodeData
            });

            // 3. 准备映射表
            const rtMap = new Map<string, string>();
            rtMap.set('inScreen', startNodeId);
            const textureMap = new Map<string, string>();

            // === 【修改点 1：更严谨的粒子节点识别】 ===
            const passOutputs = new Set(graph.map(p => p.output));

            // 收集所有可能的粒子 RT：rtPool 中的 + depthLinks 的目标中的
            const potentialParticleRTs = new Set<string>();

            // 1. 从 rtPool 收集
            data.rtPool.forEach(rt => {
                if (rt !== 'inScreen' && rt !== 'outScreen' && !passOutputs.has(rt)) {
                    potentialParticleRTs.add(rt);
                }
            });

            // 2. 从 depthLinks 收集 (防止漏网之鱼导致无法连线)
            if (data.depthLinks) {
                data.depthLinks.forEach(link => {
                    link.targets.forEach(t => {
                        if (t !== 'inScreen' && t !== 'outScreen' && !passOutputs.has(t)) {
                            potentialParticleRTs.add(t);
                        }
                    });
                });
            }

            // 创建粒子节点
            Array.from(potentialParticleRTs).forEach((rtName, index) => {
                const particleNodeId = generateId();
                rtMap.set(rtName, particleNodeId);
                newNodes.push({
                    id: particleNodeId,
                    type: 'particleNode',
                    position: { x: 50, y: 450 + index * 120 },
                    data: { name: rtName, output: rtName, themeColor: getTechColor() } as ParticleNodeData
                });
            });
            // ==========================================

            // 4. 遍历 JSON 生成 PassNode
            let currentX = 450;
            const startY = 100;

            graph.forEach((pass, index) => {
                const nodeId = generateId();
                // 记录 Output RT
                if (pass.output && pass.output !== 'outScreen') {
                    rtMap.set(pass.output, nodeId);
                }

                // 4.1 Input Ports
                const inputPorts: port[] = [];
                const inputConnections: { portId: string, sourceRt: string }[] = [];

                pass.inputs.forEach(([portName, sourceRt]) => {
                    const portId = portName; // 【建议】直接用名字做 ID，防止 ID 错乱
                    inputPorts.push({ id: portId, name: portName });
                    inputConnections.push({ portId, sourceRt });
                });

                // 4.2 Texture Ports
                const texturePorts: port[] = [];
                const textureConnections: { portId: string, textureNodeId: string }[] = [];

                if (pass.textures) {
                    Object.entries(pass.textures).forEach(([portName, path]) => {
                        const portId = portName; // 【建议】直接用名字做 ID
                        texturePorts.push({ id: portId, name: portName });

                        let texNodeId = textureMap.get(path);
                        if (!texNodeId) {
                            texNodeId = generateId();
                            textureMap.set(path, texNodeId);
                            newNodes.push({
                                id: texNodeId,
                                type: 'textureNode',
                                position: { x: currentX, y: startY + 500 + (index % 2) * 150 },
                                data: { name: 'Texture', path: path, output: path, themeColor: '#3a8ee6' } as TextureNodeData
                            });
                        }
                        textureConnections.push({ portId, textureNodeId: texNodeId });
                    });
                }

                // 4.3 创建 PassNode
                newNodes.push({
                    id: nodeId,
                    type: 'passNode',
                    position: { x: currentX, y: startY + (index % 2) * 60 },
                    data: {
                        name: pass.name,
                        vs: pass.vs,
                        fs: pass.fs,
                        inputPorts: inputPorts,
                        texturePorts: texturePorts,
                        uniforms: pass.uniforms || {},
                        output: pass.output,
                        baseColor: pass.baseColor,
                        themeColor: getTechColor()
                    } as PassNodeData
                });

                // 5. 生成连线 (Inputs & Textures)
                inputConnections.forEach(({ portId, sourceRt }) => {
                    const sourceNodeId = rtMap.get(sourceRt);
                    if (sourceNodeId) {
                        newEdges.push({
                            id: `edge_rt_${uuidv4()}`,
                            source: sourceNodeId,
                            sourceHandle: 'output',
                            target: nodeId,
                            targetHandle: portId,
                            style: { stroke: '#999', strokeWidth: 2 }
                        });
                    }
                });

                textureConnections.forEach(({ portId, textureNodeId }) => {
                    newEdges.push({
                        id: `edge_tex_${uuidv4()}`,
                        source: textureNodeId,
                        sourceHandle: 'output',
                        target: nodeId,
                        targetHandle: portId,
                        style: { stroke: '#3a8ee6', strokeWidth: 2 }
                    });
                });

                if (pass.output === 'outScreen') {
                    newEdges.push({
                        id: `edge_out_${uuidv4()}`,
                        source: nodeId,
                        sourceHandle: 'output',
                        target: endNodeId,
                        targetHandle: 'input',
                        animated: true,
                        style: { stroke: '#e84855', strokeWidth: 2.5 }
                    });
                }

                currentX += 450;
            });

            // 6. Output Node
            newNodes.push({
                id: endNodeId,
                type: 'outputNode',
                position: { x: currentX, y: 300 },
                data: { name: "Output", label: '输出: outScreen', output: 'outScreen', isStatic: true, inputPorts: [], texturePorts: [], uniforms: {} } as PassNodeData
            });

            // === 【新增点：深度共享连线 (Depth Links)】 ===
            // 这部分逻辑在你提供的代码中是缺失的，必须补上才能看到连线
            if (data.depthLinks) {
                data.depthLinks.forEach(link => {
                    const sourceNodeId = rtMap.get(link.source);

                    if (sourceNodeId) {
                        // 遍历 targets 数组，为每一个目标创建一条独立的连线
                        link.targets.forEach(targetRT => {
                            const targetNodeId = rtMap.get(targetRT);

                            if (targetNodeId) {
                                newEdges.push({
                                    id: `edge_depth_${uuidv4()}`, // 确保 ID 唯一
                                    source: sourceNodeId,
                                    sourceHandle: 'depth-out', // 固定 ID
                                    target: targetNodeId,
                                    targetHandle: 'depth-in',  // 固定 ID
                                    className: 'depth-edge',   // 样式类名
                                    style: { stroke: '#a855f7', strokeWidth: 2, strokeDasharray: '5,5' },
                                    animated: true,
                                    // 确保引入了 MarkerType，或者直接写字符串 'arrowclosed'
                                    markerEnd: { type: MarkerType.ArrowClosed, color: '#a855f7' }
                                });
                            } else {
                                console.warn(`深度连线目标未找到: ${targetRT}`);
                            }
                        });
                    } else {
                        console.warn(`深度连线源未找到: ${link.source}`);
                    }
                });
            }
            // ==========================================

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
        const { edges, nodes } = get();

        // 1. 端口占用检查（一对一逻辑）：
        // 如果目标节点的该插槽（targetHandle）已经有连线，则先过滤掉旧线
        const filteredEdges = edges.filter(
            (edge) => !(edge.target === connection.target && edge.targetHandle === connection.targetHandle)
        );

        // 2. 获取源节点以确定连线类型
        const sourceNode = nodes.find((n) => n.id === connection.source);

        // 3. 构造符合 Edge 类型的对象（不使用 any）
        const newEdge: Edge = {
            id: `e-${uuidv4()}`,
            source: connection.source,
            target: connection.target,
            sourceHandle: connection.sourceHandle,
            targetHandle: connection.targetHandle,
            // 默认样式（RT 连线）
            style: { stroke: '#999', strokeWidth: 2 },
            animated: false,
        };

        // 4. 根据业务逻辑分配特殊样式

        // 情况 A: 深度共享连线 (紫色虚线 + 箭头)
        const isDepth = connection.sourceHandle === 'depth-out' || connection.targetHandle === 'depth-in';
        if (isDepth) {
            newEdge.style = { stroke: '#a855f7', strokeWidth: 2, strokeDasharray: '5,5' };
            newEdge.animated = true;
            newEdge.className = 'depth-edge'; // 对应 CSS 动画类名
            newEdge.markerEnd = {
                type: MarkerType.ArrowClosed,
                color: '#a855f7',
            };
        }
        // 情况 B: 材质节点连线 (蓝色线)
        else if (sourceNode?.type === 'textureNode') {
            newEdge.style = { stroke: '#3a8ee6', strokeWidth: 2 };
        }
        // 情况 C: 最终输出到屏幕 (通常连向 END_NODE)
        else if (connection.target === 'END_NODE') {
            newEdge.style = { stroke: '#f87171', strokeWidth: 2.5 };
            newEdge.animated = true;
        }

        // 5. 使用 React Flow 的 addEdge 工具合并状态
        set({
            edges: addEdge(newEdge, filteredEdges)
        });
    },
    isRTNameDuplicate: (nodeId: string, name: string) => {
        // 1. 检查保留关键字
        if (name === 'inScreen') return { error: true, msg: 'inScreen 是保留输入名，不可作为输出' };
        if (name === 'outScreen') return { error: true, msg: 'outScreen 是保留输出名，自动生成的' };
        if (!name || name.trim() === '') return { error: true, msg: '名字不能为空' };

        const { nodes } = get();

        // 2. 遍历全局节点检查冲突
        const duplicateNode = nodes.find(n => {
            // A. 跳过正在编辑的这个节点自己
            if (n.id === nodeId) return false;

            // B. 检查 Pass 节点的输出 RT
            if (n.type === 'passNode') {
                const d = n.data as PassNodeData;
                return d.output === name;
            }

            // C. 检查 粒子 节点的输出 RT
            // (这一步同时覆盖了：Pass改名撞粒子、粒子改名撞Pass、粒子改名撞粒子)
            if (n.type === 'particleNode') {
                const d = n.data as ParticleNodeData;
                return d.output === name;
            }

            return false;
        });

        if (duplicateNode) {
            const typeName = duplicateNode.type === 'passNode' ? 'Pass节点' : '粒子节点';
            return {
                error: true,
                msg: `命名冲突：RT名称 "${name}" 已被另一个${typeName}使用`
            };
        }

        return { error: false };
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
                inputPorts: [{ id: `in_${generateId()}`, name: 'u_rt0' }],
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

        nodes.forEach(n => {
            if (n.type === 'particleNode') {
                rtPoolSet.add((n.data as ParticleNodeData).output);
            }
            // 注意：这里没有添加 textureNode 的输出
        });

        // === 【新增点 2】：提取深度绑定逻辑 ===
        const depthLinksMap = new Map<string, Set<string>>();
        edges.forEach(e => {
            if (e.sourceHandle === 'depth-out' && e.targetHandle === 'depth-in') {
                const srcNode = nodes.find(n => n.id === e.source);
                const tarNode = nodes.find(n => n.id === e.target);

                if (srcNode && tarNode) {
                    // 类型安全地获取 RT 名称
                    let srcRT = '';
                    if (srcNode.type === 'inputNode' || srcNode.type === 'passNode') {
                        srcRT = (srcNode.data as PassNodeData).output;
                    } else if (srcNode.type === 'particleNode') {
                        srcRT = (srcNode.data as ParticleNodeData).output;
                    }

                    let tarRT = '';
                    if (tarNode.type === 'passNode') {
                        tarRT = (tarNode.data as PassNodeData).output;
                    } else if (tarNode.type === 'particleNode') {
                        tarRT = (tarNode.data as ParticleNodeData).output;
                    }

                    if (srcRT && tarRT) {
                        if (!depthLinksMap.has(srcRT)) depthLinksMap.set(srcRT, new Set());
                        depthLinksMap.get(srcRT)!.add(tarRT);
                    }
                }
            }
        });

        const depthLinks = Array.from(depthLinksMap.entries()).map(([source, targets]) => ({
            source,
            targets: Array.from(targets)
        }));

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

                    let sourceOutput = '';
                    if (sourceNode.type === 'textureNode') {
                        sourceOutput = (sourceNode.data as TextureNodeData).output;
                    } else if (sourceNode.type === 'particleNode') {
                        sourceOutput = (sourceNode.data as ParticleNodeData).output;
                    } else {
                        sourceOutput = (sourceNode.data as PassNodeData).output;
                    }
                    return [port.name || 'u_texture', sourceOutput] as [string, string];
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

        return { postProcess: { name: postProcessName, rtPool: Array.from(rtPoolSet), depthLinks: depthLinks, hint: "拓扑序", postProcessGraph: sortedPasses } };
    }
}));