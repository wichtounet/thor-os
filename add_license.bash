for file in "$@"
do
    lines=`grep "Distributed under the Boost Software License" $file | wc -l`

    if [[ $lines == 0 ]]
    then
        cp ${file} ${file}.orig

        asm=`echo $file | grep ".asm" |  wc -l`

        if [[ $asm == 0 ]]
        then
            cat cpp_license_header ${file}.orig > ${file}
        else
            cat asm_license_header ${file}.orig > ${file}
        fi

        rm ${file}.orig
    fi
done
