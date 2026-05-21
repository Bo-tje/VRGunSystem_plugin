import os
import re

source_dir = r"c:\Users\bo\Desktop\git\VRGunSystem_plugin\GunSystemDevelopment\Plugins\VRModularWeaponSystem\Source\VRModularWeaponSystem"
public_dir = os.path.join(source_dir, "Public")
private_dir = os.path.join(source_dir, "Private")

print("Walking source directories...")
files_to_check = []
for root, dirs, files in os.walk(source_dir):
    for file in files:
        if file.endswith(('.h', '.cpp')):
            files_to_check.append(os.path.join(root, file))

print(f"Found {len(files_to_check)} files.")

# Check for specific patterns:
# 1. Tick enabled (bCanEverTick = true in constructor, and/or Tick functions)
# 2. Raw pointers used where TObjectPtr might be expected (or vice versa in local scopes, etc.)
# 3. Virtual functions without 'override'
# 4. Empty or boilerplate code
# 5. Optimization spots (e.g. FindComponentByClass in ticks, or frequent casts)

report = []

for filepath in files_to_check:
    rel_path = os.path.relpath(filepath, source_dir)
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    lines = content.splitlines()
    line_count = len(lines)
    
    file_issues = []
    
    # 1. Check for Tick
    if "bCanEverTick = true" in content:
        # Check if Tick function is actually overridden or defined
        if "::Tick" not in content and "TickComponent" not in content:
            file_issues.append("bCanEverTick is set to true in constructor, but no Tick function is implemented or used.")
        else:
            file_issues.append("Tick is enabled. Verify if Tick is actually needed or if it can be replaced with event-based logic.")
            
    # 2. FindComponentByClass in loops or frequent calls
    # (Just a general check for now)
    if "GetComponentByClass" in content or "FindComponentByClass" in content:
        # Check if it's inside a Tick function (simple substring check)
        tick_func = re.findall(r'void\s+\w+::Tick(?:Component)?\(.*?\)\s*\{(.*?)\}', content, re.DOTALL)
        for tf in tick_func:
            if "GetComponentByClass" in tf or "FindComponentByClass" in tf:
                file_issues.append("GetComponentByClass/FindComponentByClass called inside Tick. Consider caching this component in BeginPlay.")
                
    # 3. Cast in Tick
    if "Cast<" in content:
        tick_func = re.findall(r'void\s+\w+::Tick(?:Component)?\(.*?\)\s*\{(.*?)\}', content, re.DOTALL)
        for tf in tick_func:
            if "Cast<" in tf:
                file_issues.append("Cast<> used in Tick. Consider caching the casted pointer in BeginPlay.")

    # 4. Check for TObjectPtr usage in headers vs private cpp files
    # (In UE5, UPROPERTY of class types should ideally use TObjectPtr in headers)
    if filepath.endswith('.h'):
        raw_uproperties = re.findall(r'UPROPERTY\([^)]*\)\s*([\w_]+)\*\s+([\w_]+);', content)
        for type_name, var_name in raw_uproperties:
            if not type_name.startswith('F') and type_name != 'void':
                file_issues.append(f"Raw pointer UPROPERTY: '{type_name}* {var_name}'. In UE5, TObjectPtr<{type_name}> is preferred for member variables.")

    # 5. Check for missing override on standard UE functions (BeginPlay, Tick, EndPlay)
    if filepath.endswith('.h'):
        for func in ['BeginPlay', 'Tick', 'TickComponent', 'EndPlay']:
            if re.search(r'\b' + func + r'\b', content) and not re.search(r'\b' + func + r'\b.*?\boverride\b', content) and not "DECLARE_FUNCTION" in content:
                # Make sure it's not a comment
                # Just a simple heuristic:
                match = re.search(r'virtual\s+void\s+' + func + r'\b', content)
                if match:
                    # check if override is present in that line or the next few
                    substr = content[match.start():match.start()+100]
                    if "override" not in substr:
                        file_issues.append(f"Virtual function '{func}' is missing 'override' specifier.")

    if file_issues:
        report.append((rel_path, line_count, file_issues))

print("\n--- Project Analysis Report ---")
for rel_path, lines, issues in sorted(report, key=lambda x: len(x[2]), reverse=True):
    print(f"\nFile: {rel_path} ({lines} lines)")
    for issue in issues:
        print(f" - {issue}")
