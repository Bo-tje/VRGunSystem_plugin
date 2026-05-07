import os
import re
import json

base_path = r'c:\Users\bo\Desktop\git\VRGunSystem_plugin\GunSystemDevelopment\Plugins\VRModularWeaponSystem\Source\VRModularWeaponSystem\Public'

output = {}

for root, dirs, files in os.walk(base_path):
    category = os.path.basename(root)
    if category == 'Public':
        continue
    
    if category not in output:
        output[category] = []
        
    for file in files:
        if file.endswith('.h'):
            filepath = os.path.join(root, file)
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Find the main class definition
            # Usually UCLASS() class MACRO_API Name : public Base
            main_class = None
            uclass_match = re.search(r'(?:UCLASS|USTRUCT|UINTERFACE)\([^)]*\)\s*(?:class|struct)\s+(?:[A-Z0-9_]+_API\s+)?([A-Z]\w+)', content)
            if uclass_match:
                main_class = uclass_match.group(1)
            else:
                # Fallback
                match = re.search(r'(?:class|struct)\s+(?:[A-Z0-9_]+_API\s+)?([A-Z]\w+)\s*:\s*public', content)
                if match:
                    main_class = match.group(1)
            
            if not main_class:
                continue
                
            props = []
            prop_matches = re.finditer(r'UPROPERTY\([^)]*\)\s*(?:[\w<>*]+\s+)*([\w<>*]+)\s+(\w+)', content)
            for m in prop_matches:
                t = m.group(1)
                n = m.group(2)
                props.append(f"{t} {n}")
                
            funcs = []
            func_matches = re.finditer(r'UFUNCTION\([^)]*\)\s*(?:virtual\s+)?([\w<>*]+)\s+(\w+)\(', content)
            for m in func_matches:
                t = m.group(1)
                n = m.group(2)
                funcs.append(f"{n}()")
                
            delegates = []
            del_matches = re.finditer(r'DECLARE_DYNAMIC_MULTICAST_DELEGATE[^\(]*\(\s*(\w+)', content)
            for m in del_matches:
                delegates.append(m.group(1))
                
            output[category].append({
                'name': main_class,
                'file': file,
                'props': list(set(props)),
                'funcs': list(set(funcs)),
                'delegates': list(set(delegates))
            })

with open('full_scan.json', 'w', encoding='utf-8') as f:
    json.dump(output, f, indent=4)
