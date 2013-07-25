
copy_example_namedpin() {
        mkdir -p pinning/named_pin
  	if [ ! -f "pinning/named_pin/default" ]
	then
		echo "Copy example named-pin file"
		cp "core/portio/example-config" "pinning/named_pin/default" > /dev/null
	fi
}

