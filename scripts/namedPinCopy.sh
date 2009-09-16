
copy_example_namedpin() {
  	if [ ! -f "namedpinConfig.user" ]
	then
		echo "Copy example named-pin file"
		cp "core/portio/example-config" "namedpinConfig.user" > /dev/null
	fi
}

