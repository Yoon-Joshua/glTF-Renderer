import subprocess
import json
import os
import copy

VK_SHADER_STAGE_VERTEX_BIT = 1,
VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT = 2,
VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 4,
VK_SHADER_STAGE_GEOMETRY_BIT = 8,
VK_SHADER_STAGE_FRAGMENT_BIT = 16,
VK_SHADER_STAGE_COMPUTE_BIT = 32,

stage_to_bit={
    "vert":1,
    "tesc":2,
    "tese":4,
    "geom":8,
    "frag":16,
}

vec_size_dict={
    "vec2":2,
    "vec3":3,
    "vec4":4
}

type_to_size={
    "float":4,
    "vec4":16
}

# 调用可执行程序
def run_exe_command(command:list,waitTime=10000):
    p=subprocess.Popen(command)
    try:
        p.wait(timeout=1000)
    except Exception as e:
        print("===== process timeout {} =====".format(command))
        p.kill()

def get_push_constant_size(types,push_constant):
    type=push_constant["type"]
    type=types[type]
    members=type["members"]
    highest_offset=0
    index=0
    for i in range(len(members)):
        if members[i]["offset"]>highest_offset:
            highest_offset=members[i]["offset"]
            index=i
    
    return highest_offset+type_to_size[members[index]["type"]]

# 将包含资源信息的json文件转换为cpp程序需要的格式
def shift_resource(res_file):
    f=open(res_file,mode='r',encoding='utf8')
    data=json.load(f)
    f.close()

    entry_points=data["entryPoints"]
    assert(len(entry_points)==1)
    stage=entry_points[0]["mode"]

    resources=[]
    # Input
    inputs=data["inputs"] if "inputs" in data else []
    for input in inputs:
        res={}
        res["stages"]=stage_to_bit[stage]
        res["type"]="Input"
        res["mode"]="Static"
        res["set"]=0
        res["binding"]=0
        res["location"]=input["location"]
        res["input_attachment_index"]=0
        res["vec_size"]=vec_size_dict[input["type"]]
        res["columns"]=1
        res["array_size"]=1
        res["offset"]=0
        res["size"]=0
        res["constant_id"]=0
        res["qualifiers"]=0
        res["name"]=input["name"]
        resources.append(res)
    # InputAttachment
    input_attachments=data["subpass_inputs"] if "subpass_inputs" in data else []
    for input in input_attachments:
        res={}
        res["stages"]=stage_to_bit[stage]
        res["type"]="InputAttachment"
        res["mode"]="Static"
        res["set"]=input["set"]
        res["binding"]=input["binding"]
        res["location"]=0
        res["input_attachment_index"]=input["input_attachment_index"]
        res["vec_size"]=0
        res["columns"]=0
        res["array_size"]=1
        res["offset"]=0
        res["size"]=0
        res["constant_id"]=0
        res["qualifiers"]=0
        res["name"]=input["name"]
        resources.append(res)
    # Output
    outputs=data["outputs"]
    for output in outputs:
        res={}
        res["stages"]=stage_to_bit[stage]
        res["type"]="Output"
        res["mode"]="Static"
        res["set"]=0
        res["binding"]=0
        res["location"]=output["location"]
        res["input_attachment_index"]=0
        res["vec_size"]=vec_size_dict[output["type"]]
        res["columns"]=1
        res["array_size"]=1
        res["offset"]=0
        res["size"]=0
        res["constant_id"]=0
        res["qualifiers"]=0
        res["name"]=output["name"]
        resources.append(res)
    # ImageSampler
    samplers=data["textures"] if "textures" in data else []
    for sampler in samplers:
        res={}
        res["stages"]=stage_to_bit[stage]
        res["type"]="ImageSampler"
        res["mode"]="Static"
        res["set"]=sampler["set"]
        res["binding"]=sampler["binding"]
        res["location"]=0
        res["input_attachment_index"]=0
        res["vec_size"]=0
        res["columns"]=0
        res["array_size"]=1
        res["offset"]=0
        res["size"]=0
        res["constant_id"]=0
        res["qualifiers"]=0
        res["name"]=sampler["name"]
        resources.append(res)
    # BufferUniform
    ubos=data["ubos"] if "ubos" in data else []
    for ubo in ubos:
        res={}
        res["stages"]=stage_to_bit[stage]
        res["type"]="BufferUniform"
        res["mode"]="Static"
        res["set"]=ubo["set"]
        res["binding"]=ubo["binding"]
        res["location"]=0
        res["input_attachment_index"]=0
        res["vec_size"]=0
        res["columns"]=0
        res["array_size"]=1
        res["offset"]=0
        res["size"]=ubo["block_size"]
        res["constant_id"]=0
        res["qualifiers"]=0
        res["name"]=ubo["name"]
        resources.append(res)
    # PushConstant
    push_constants=data["push_constants"] if "push_constants" in data else []
    for push_constant in push_constants:
        res={}
        res["stages"]=stage_to_bit[stage]
        res["type"]="PushConstant"
        res["mode"]="Static"
        res["set"]=0
        res["binding"]=0
        res["location"]=0
        res["input_attachment_index"]=0
        res["vec_size"]=0
        res["columns"]=0
        res["array_size"]=0
        res["offset"]=0
        res["size"]=get_push_constant_size(data["types"],push_constant)
        res["constant_id"]=0
        res["qualifiers"]=0
        res["name"]=push_constant["name"]
        resources.append(res)
    
    specialization_constants = data["specialization_constants"] if "specialization_constants" in data else []
    for specialization_constant in specialization_constants:
        res={}
        res["stages"]=stage_to_bit[stage]
        res["type"]="SpecializationConstant"
        res["mode"]="Static"
        res["set"]=0
        res["binding"]=0
        res["location"]=0
        res["input_attachment_index"]=0
        res["vec_size"]=0
        res["columns"]=0
        res["array_size"]=0
        res["offset"]=0
        res["size"]=4
        res["constant_id"]=specialization_constant['id']
        res["qualifiers"]=0
        res["name"]=specialization_constant["name"]
        resources.append(res)

    
    output={"resources":resources}
    json_str = json.dumps(output)
    f=open(res_file,mode='w',encoding='utf8')
    f.write(json_str)
    f.close()

def backtrace(i,macros,types,result,cur):
    if i>=len(macros):
        result.append(copy.copy(cur))
        return
    if types[i]==0:
        for j in range(pow(2,len(macros[i]))):
            cur.append(j)
            backtrace(i+1,macros,types,result,cur)
            cur.pop()
    else:
        for j in range(len(macros[i])):
            cur.append(pow(2,j))
            backtrace(i+1,macros,types,result,cur)
            cur.pop()

def compile_all(source_path):
    source_path=os.path.abspath(source_path)
    json_file=os.path.splitext(source_path)[0]+'.macro.json'
    f=open(json_file,encoding='utf8',mode='r')
    data=json.load(f)
    f.close()

    macros=[]
    types=[]
    for key in data["all"]:
        macros.append(data[key]['macros'])
        types.append(data[key]['type'])
    
    result=[]
    cur=[]
    backtrace(0,macros,types,result,cur)
    
    bin_dir=source_path+".bin2"
    if not os.path.exists(bin_dir):
        os.mkdir(bin_dir)
    
    res_dir=source_path+".res2"
    if not os.path.exists(res_dir):
        os.mkdir(res_dir)
    
    for r in result:
        # 生成spirv文件
        basename=''
        cmd=["glslangValidator","-V", source_path]
        for i in range(len(r)):
            defined=bin(r[i])[2:].rjust(32,'0')
            defined=defined[::-1]

            basename=basename+str(r[i])+'.'
            
            for j in range(len(defined)):
                if j>len(defined):
                    break
                if defined[j]=='1':
                    cmd.append("-D"+macros[i][j])
        
        output=os.path.join(bin_dir,basename+'spv')
        cmd.append('-o')
        cmd.append(output)
        cmd_str=""
        for word in cmd:
            cmd_str=cmd_str+word+" "
        cmd_str+=" "
        print(cmd_str)
        run_exe_command(cmd)

        # 生成包含资源信息的json文件
        resource=os.path.join(res_dir,basename+'json')
        cmd=["spirv-cross",output,"--reflect","--output",resource]
        run_exe_command(cmd)

        #将包含资源信息的json文件转换为cpp程序需要的格式
        shift_resource(resource)


if __name__=='__main__':
    compile_all('shaders/deferred/lighting.frag')

    # config_file="config/shader.json"
    # f=open(config_file,mode='r',encoding='utf8')
    # data=json.load(f)
    # f.close()

    # attribute = data["attribute_macro"]
    # binding = data["binding_macro"]

    # source=os.path.abspath("shaders/deferred/geometry.vert")
    # compile_all(source,attribute,binding)
