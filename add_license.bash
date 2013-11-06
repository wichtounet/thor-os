for file in "$@"
do
    cp ${file} ${file}.orig
    cat license_header ${file}.orig > ${file}
    rm ${file}.orig
done
