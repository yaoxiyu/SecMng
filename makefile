inc_path = ./inc
obj_path = ./obj
src_path = ./src
src = $(wildcard $(src_path)/*.c)
obj = $(patsubst $(src_path)/%.c, $(obj_path)/%.o, $(src))
target = ./bin/app

ALL:$(target)

$(target):$(obj)
	gcc $^ -o $@
	  
$(obj):$(obj_path)/%.o:$(src_path)/%.c
	gcc -c $< -o $@ -I $(inc_path)

clean:
	-rm -rf $(obj) $(target)

.PHONY: clean ALL
