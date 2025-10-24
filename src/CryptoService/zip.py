import zipfile

# @param src: list of files to zip
# @param dest: archive name
def zip(src: list[str], dest: str):
    with zipfile.ZipFile(dest, "w", zipfile.ZIP_DEFLATED) as zip_file:
        for file in src:
            zip_file.write(file)

# @param src: archive
# @param dest: target directory
def unzip(src: str, dest:str):
    with zipfile.ZipFile(src, "r") as zip_file:
        zip_file.extractall(dest)