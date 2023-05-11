OUTPUT=gitupdater

$(OUTPUT): main.c
	gcc -lc $^ -o $@

clean:
	rm -rf $(OUTPUT)