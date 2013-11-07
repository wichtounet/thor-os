for file in "$@"
do
    lines=`grep "Distributed under the Boost Software License" $file | wc -l`

    if [[ $lines == 0 ]]
    then
        cp ${file} ${file}.orig
        cat license_header ${file}.orig > ${file}
        rm ${file}.orig
    fi
done
