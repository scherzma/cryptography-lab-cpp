def process_json(input_file, output_file):
    import re
    from collections import defaultdict

    # Read the file as raw text
    with open(input_file, 'r') as f:
        content = f.read()

    # Keep track of key occurrences
    key_counts = defaultdict(int)

    # Find all keys and count them
    pattern = r'"([^"]+)":'
    for match in re.finditer(pattern, content):
        key = match.group(1)
        key_counts[key] += 1

    # Reset counts for the actual replacement
    current_counts = defaultdict(int)

    # Function to handle each key match
    def replace_key(match):
        key = match.group(1)
        current_counts[key] += 1

        # Only add number if it's a duplicate
        if key_counts[key] > 1:
            return f'"{key}{current_counts[key]}":'
        return match.group(0)

    # Replace the keys
    modified_content = re.sub(pattern, replace_key, content)

    # Write the modified content
    with open(output_file, 'w') as f:
        f.write(modified_content)


# Example usage
if __name__ == "__main__":
    input_file = "gcm_output_uni.json"
    output_file = "gcm_output_long.json"
    process_json(input_file, output_file)