#!/bin/bash

set -e

# Function to get items from environment variables
get_env_items() {
    local var_name=$1
    local value="${!var_name}"
    echo "$value"
}

# Function to remove JIRA numbers from items
remove_jira_numbers() {
    echo "$1" | sed -E 's/\[[A-Za-z0-9_-]+\](\([^)]+\))?//g' | sed 's/[[:space:]]*$//'
}

# Function to update release notes section (no version bump)
update_section() {
    local file=$1
    local section_number=$2
    local items=$3
    local remove_jira=$4
    
    if [[ -z "$items" ]]; then
        echo "DEBUG: No items for section $section_number"
        return 0
    fi
    
    echo "DEBUG: Updating section $section_number with items"
    
    # Find the section header line
    local section_line=$(grep -n "^### ${section_number}\." "$file" | head -n 1 | cut -d: -f1)
    
    if [[ -z "$section_line" ]]; then
        echo "ERROR: Could not find section ### ${section_number}. in $file"
        return 1
    fi
    
    echo "DEBUG: Found section $section_number at line $section_line"
    
    # Determine next section line based on section number
    local next_section_line
    case $section_number in
        1) 
            next_section_line=$(grep -n '^### 2\. Fixed' "$file" | head -n 1 | cut -d: -f1)
            if [[ -n "$next_section_line" ]]; then
                next_section_line=$((next_section_line - 1))
            fi
            ;;
        2) 
            next_section_line=$(grep -n '^### 3\. Added' "$file" | head -n 1 | cut -d: -f1)
            if [[ -n "$next_section_line" ]]; then
                next_section_line=$((next_section_line - 1))
            fi
            ;;
        3) 
            next_section_line=$(grep -n '^## v' "$file" | tail -n +2 | head -n 1 | cut -d: -f1)
            if [[ -n "$next_section_line" ]]; then
                next_section_line=$((next_section_line - 2))
            fi
            ;;
    esac
    
    # If we can't find the next section, use end of file
    if [[ -z "$next_section_line" ]]; then
        next_section_line=$(wc -l < "$file")
    fi
    
    echo "DEBUG: Next section at line $next_section_line"
    
    # Process items (remove JIRA numbers if requested)
    local processed_items="$items"
    if [[ "$remove_jira" == "true" ]]; then
        processed_items=$(remove_jira_numbers "$items")
    fi
    
    # Create temporary file with new content
    local temp_file="temp_section_${section_number}.md"
    
    # Add new items first
    echo "$processed_items" > "$temp_file"
    
    # Add existing content from the section (excluding the header line)
    sed -n "$((section_line + 1)),${next_section_line}p" "$file" >> "$temp_file"
    
    # Replace the section content in the original file
    # Delete the old section content
    sed -i "$((section_line + 1)),${next_section_line}d" "$file"
    
    # Insert the new content
    sed -i "${section_line}r $temp_file" "$file"
    
    # Clean up
    rm -f "$temp_file"
    
    echo "DEBUG: Updated section $section_number successfully"
}

# Function to create new version (when bump occurred)
create_new_version() {
    local new_version=$1
    local changed_items=$2
    local fixed_items=$3
    local added_items=$4
    
    local new_version_header="## v${new_version} / $(date +'%Y-%m-%d')"
    echo "Creating new version: $new_version_header"
    
    local temp_file=$(mktemp)
    
    # Create new version content
    {
        echo "# RELEASE_NOTES"
        echo "$new_version_header"
        echo
        echo "### 1. Changed"
        if [[ -n "$changed_items" ]]; then
            remove_jira_numbers "$changed_items"
        fi
        echo
        echo "### 2. Fixed"
        if [[ -n "$fixed_items" ]]; then
            remove_jira_numbers "$fixed_items"
        fi
        echo
        echo "### 3. Added"
        if [[ -n "$added_items" ]]; then
            remove_jira_numbers "$added_items"
        fi
        echo
    } > "$temp_file"
    
    # Append existing content if file exists
    if [[ -f RELEASE_NOTES.md ]]; then
        local line_num=$(grep -n "^# RELEASE_NOTES$" RELEASE_NOTES.md | head -1 | cut -d: -f1)
        if [[ -n "$line_num" ]]; then
            tail -n +$((line_num + 1)) RELEASE_NOTES.md >> "$temp_file"
        else
            cat RELEASE_NOTES.md >> "$temp_file"
        fi
    fi
    
    # Move temp file to final location
    mv "$temp_file" RELEASE_NOTES.md
    
    echo "✅ Created new version release notes"
}

# Main execution
main() {
    local bump_type=$1
    local new_version=$2
    
    # Get items from environment variables
    echo "Getting items from environment variables..."
    local changed_items=$(get_env_items "CHANGED_ITEMS")
    local fixed_items=$(get_env_items "FIXED_ITEMS")
    local added_items=$(get_env_items "ADDED_ITEMS")
    
    # Debug output
    echo "Changed items: ${#changed_items} chars"
    echo "Fixed items: ${#fixed_items} chars"
    echo "Added items: ${#added_items} chars"
    
    if [[ "$bump_type" == "none" ]]; then
        echo "No version bump - updating existing release notes..."
        
        # Update RELEASE_NOTES.md (without JIRA numbers)
        if [[ -f RELEASE_NOTES.md ]]; then
            echo "Updating RELEASE_NOTES.md..."
            
            # Get current line numbers (recalculate after each update)
            local first_changed_line=$(grep -n '^### 1\. Changed' RELEASE_NOTES.md | head -n 1 | cut -d: -f1)
            local first_fixed_line=$(grep -n '^### 2\. Fixed' RELEASE_NOTES.md | head -n 1 | cut -d: -f1)
            local first_added_line=$(grep -n '^### 3\. Added' RELEASE_NOTES.md | head -n 1 | cut -d: -f1)
            local second_version_line=$(grep -n '^## v' RELEASE_NOTES.md | tail -n +2 | head -n 1 | cut -d: -f1)
            
            echo "Line numbers - Changed: $first_changed_line, Fixed: $first_fixed_line, Added: $first_added_line, Second version: $second_version_line"
            
            # Update Changed section
            if [[ -n "$changed_items" ]]; then
                local next_section_line=$((first_fixed_line - 1))
                local changed_items_removed_jira=$(remove_jira_numbers "$changed_items")
                echo "$changed_items_removed_jira" > temp_changed.md
                sed -n "$((first_changed_line + 1)),${next_section_line}p" RELEASE_NOTES.md >> temp_changed.md
                sed -i "$((first_changed_line + 1)),${next_section_line}d" RELEASE_NOTES.md
                sed -i "${first_changed_line}r temp_changed.md" RELEASE_NOTES.md
            fi
            
            # Recalculate line numbers after Changed section update
            first_fixed_line=$(grep -n '^### 2\. Fixed' RELEASE_NOTES.md | head -n 1 | cut -d: -f1)
            first_added_line=$(grep -n '^### 3\. Added' RELEASE_NOTES.md | head -n 1 | cut -d: -f1)
            
            # Update Fixed section
            if [[ -n "$fixed_items" ]]; then
                local next_section_line=$((first_added_line - 1))
                local fixed_items_removed_jira=$(remove_jira_numbers "$fixed_items")
                echo "$fixed_items_removed_jira" > temp_fixed.md
                sed -n "$((first_fixed_line + 1)),${next_section_line}p" RELEASE_NOTES.md >> temp_fixed.md
                sed -i "$((first_fixed_line + 1)),${next_section_line}d" RELEASE_NOTES.md
                sed -i "${first_fixed_line}r temp_fixed.md" RELEASE_NOTES.md
            fi
            
            # Recalculate line numbers after Fixed section update
            first_added_line=$(grep -n '^### 3\. Added' RELEASE_NOTES.md | head -n 1 | cut -d: -f1)
            second_version_line=$(grep -n '^## v' RELEASE_NOTES.md | tail -n +2 | head -n 1 | cut -d: -f1)
            
            # Update Added section
            if [[ -n "$added_items" ]]; then
                local next_section_line=$((second_version_line - 2))
                local added_items_removed_jira=$(remove_jira_numbers "$added_items")
                echo "$added_items_removed_jira" > temp_added.md
                sed -n "$((first_added_line + 1)),${next_section_line}p" RELEASE_NOTES.md >> temp_added.md
                sed -i "$((first_added_line + 1)),${next_section_line}d" RELEASE_NOTES.md
                sed -i "${first_added_line}r temp_added.md" RELEASE_NOTES.md
            fi
            
            # Clean up temp files
            rm -f temp_*.md
            
            echo "✅ Updated RELEASE_NOTES.md"
            
            echo "Updated RELEASE_NOTES.md:"
            cat RELEASE_NOTES.md
        fi
        
    else
        echo "Version bump detected - creating new version..."
        if [[ -z "$new_version" ]]; then
            echo "Error: NEW_VERSION is required when bump occurred"
            exit 1
        fi
        
        create_new_version "$new_version" "$changed_items" "$fixed_items" "$added_items"
    fi
    
    echo "🎉 Release notes update completed successfully!"
}

# Script usage
usage() {
    echo "Usage: $0 <bump_type> [new_version]"
    echo "  bump_type: 'none', 'patch', 'minor', or 'major'"
    echo "  new_version: required when bump_type is not 'none'"
    echo
    echo "Examples:"
    echo "  $0 'none'                         # Update existing version"
    echo "  $0 'minor' '1.2.0'               # Create new version 1.2.0"
    echo "  $0 'patch' '1.1.1'               # Create new version 1.1.1"
}

# Check arguments
if [[ $# -lt 1 ]]; then
    usage
    exit 1
fi

if [[ "$1" != "none" && $# -lt 2 ]]; then
    echo "Error: NEW_VERSION is required when bump occurred"
    usage
    exit 1
fi

# Run main function
main "$1" "$2"
