inc_path = ../inc
obj_path = ../obj
src_path = ../src
src = $(wildcard $(src_path)/*.c)
obj = $(patsubst $(src_path)/%.c, $(obj_path)/%.o, $(src))
target = ./libmymessage.so

ALL:$(target)

$(target):$(obj)
	gcc -shared $^ -o $@
	  
$(obj):$(obj_path)/%.o:$(src_path)/%.c
	gcc -c $< -o $@ -I $(inc_path) -fPIC

clean:
	-rm -rf $(obj) $(target)

.PHONY: clean ALL
