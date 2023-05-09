OUTPUT=gitupdater

$(OUTPUT): main.c
	gcc $^ -o $@

clean:
	rm -rf $(OUTPUT)