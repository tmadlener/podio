#!/usr/bin/env python3

import os

TEMPLATE = r'<dd><a href="../{version}/index.html">{version}</a></dd>'


def main(args):
    """Main"""
    versions = [
        d for d in os.listdir(args.main_dir) if os.path.isdir(f"{args.main_dir}/{d}")
    ]
    print(f"Found the following versions: {versions}")
    version_patch_lines = [TEMPLATE.format(version=v) for v in versions]

    patched_lines = []
    with open(args.inputfile, "r", encoding="utf-8") as infile:
        for line in infile.readlines():
            patched_lines.append(line.rstrip())

    start_patch = patched_lines.index(r"<!-- VERSION_CHANGE START -->")
    end_patch = patched_lines.index(r"<!-- VERSION_CHANGE END -->")
    del patched_lines[start_patch + 1 : end_patch]

    current_line = start_patch + 1
    for line in version_patch_lines:
        patched_lines.insert(current_line, line)
        current_line += 1

    with open(args.inputfile, "w", encoding="utf-8") as outfile:
        outfile.write("\n".join(patched_lines))


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Small script to update the"
        " version selectors in existing index.html files"
    )
    parser.add_argument("inputfile", help="The input file to patch")
    parser.add_argument(
        "main_dir",
        help="The main directory from which to detect " "the versions",
        default=".",
        nargs="?",
    )

    args = parser.parse_args()
    main(args)
