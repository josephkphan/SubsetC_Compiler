# Creating Symlinks
cd workspace/
mkdir $1
cd $1
for file in ../../deliverables/$1/*;
do
    echo "Creating Symlink for ${file}"
    ln -s ${file} ./
done