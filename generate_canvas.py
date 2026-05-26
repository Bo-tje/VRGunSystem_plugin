import os
import glob
import re
import json
import math

base_path = r'c:\Users\bo\Desktop\git\VRGunSystem_plugin\GunSystemDevelopment\Plugins\VRModularWeaponSystem\Source\VRModularWeaponSystem\Public'

groups = {}
# Categories to colors
color_map = {
    'Core': '1',
    'Components': '2',
    'Interaction': '3',
    'Data': '4',
    'StateTree': '5',
    'Interfaces': '6'
}

for root, dirs, files in os.walk(base_path):
    category = os.path.basename(root)
    if category == 'Public':
        continue
    if category not in groups:
        groups[category] = []
    
    for file in files:
        if file.endswith('.h'):
            filepath = os.path.join(root, file)
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Find class or struct definitions (filtering out forward declarations by requiring colon or brace)
            class_names = []
            for match in re.finditer(r'(?:class|struct)\s+(?:[A-Z0-9_]+_API\s+)?([A-Z]\w+)\s*(?::|{)', content):
                name = match.group(1)
                if name not in class_names and not name.endswith('API'):
                    class_names.append(name)
            
            if not class_names:
                continue
                
            primary_class = class_names[0]
            if primary_class.startswith('IVR') or primary_class.startswith('UVR'):
                pass # good
            elif 'Task' in primary_class or 'InstanceData' in primary_class or 'Eval' in primary_class:
                pass
            else:
                pass
            
            # get properties and functions
            props = []
            for line in content.split('\n'):
                line = line.strip()
                if 'UPROPERTY' in line or 'UFUNCTION' in line or 'DECLARE_DYNAMIC_MULTICAST_DELEGATE' in line:
                    continue # skip macro itself if split across lines
                
                # Match property: type name;
                # It's usually after UPROPERTY()
                # Actually, simple string search is hard.
                pass
            
            # Let's do a better regex for properties and functions
            props = []
            funcs = []
            
            # Regex for properties: type name;
            prop_matches = re.finditer(r'UPROPERTY\([^)]*\)\s*(?:[\w<>*]+\s+)*([\w<>*]+)\s+(\w+)', content)
            for m in prop_matches:
                t = m.group(1)
                n = m.group(2)
                props.append(f"{t} {n}")
                
            func_matches = re.finditer(r'UFUNCTION\([^)]*\)\s*(?:virtual\s+)?([\w<>*]+)\s+(\w+)\(', content)
            for m in func_matches:
                t = m.group(1)
                n = m.group(2)
                funcs.append(f"{n}()")
                
            delegates = []
            del_matches = re.finditer(r'DECLARE_DYNAMIC_MULTICAST_DELEGATE[^\(]*\(\s*(\w+)', content)
            for m in del_matches:
                delegates.append(m.group(1))

            groups[category].append({
                'name': primary_class,
                'file': file,
                'props': list(set(props)),
                'funcs': list(set(funcs)),
                'delegates': list(set(delegates))
            })

nodes = []
edges = []

x_offset = -1500
y_offset = -1000

group_width = 1200
group_spacing = 200

current_x = x_offset
current_y = y_offset

node_id_counter = 1

for category, classes in groups.items():
    if not classes:
        continue
        
    g_x = current_x
    g_y = current_y
    g_w = group_width
    g_h = math.ceil(len(classes) / 3) * 450 + 100
    
    nodes.append({
        "id": f"g_{category}",
        "type": "group",
        "label": category,
        "x": g_x,
        "y": g_y,
        "width": g_w,
        "height": g_h,
        "color": color_map.get(category, '1')
    })
    
    cx = g_x + 50
    cy = g_y + 80
    col = 0
    
    for cls in classes:
        text = f"### {cls['name']}\n"
        if cls['delegates']:
            text += "**Delegates:**\n" + ", ".join(cls['delegates']) + "\n\n"
        if cls['props']:
            text += "**Properties:**\n" + "\\n".join([f"- {p}" for p in cls['props'][:10]]) + ("..." if len(cls['props'])>10 else "") + "\n\n"
        if cls['funcs']:
            text += "**Functions:**\n" + "\\n".join([f"- {f}" for f in cls['funcs'][:10]]) + ("..." if len(cls['funcs'])>10 else "")
            
        nodes.append({
            "id": f"n_{cls['name']}",
            "type": "text",
            "text": text.strip(),
            "x": cx,
            "y": cy,
            "width": 320,
            "height": 380
        })
        
        col += 1
        cx += 360
        if col >= 3:
            col = 0
            cx = g_x + 50
            cy += 420
            
    current_y += g_h + group_spacing

data = {
    "nodes": nodes,
    "edges": edges
}

out_file = r'c:\Users\bo\Desktop\git\VRGunSystem_plugin\minor skilled notes\System design\System design chart (in-development).canvas'
with open(out_file, 'w', encoding='utf-8') as f:
    json.dump(data, f, indent=4)

print("Canvas generated successfully.")
