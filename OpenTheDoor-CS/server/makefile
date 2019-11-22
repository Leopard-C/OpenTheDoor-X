
TARGET := door_server

CC := g++

SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
DEPS_DIR = $(BUILD_DIR)/deps

INC_DIR = -I./include

CC_FLAGS := $(INC_DIR) -std=c++14
LNK_FLAGS := -ljsoncpp \
						 -lpthread \
						 -lwiringPi \
						 -lssl \
						 -lcurl \
						 -lcrypto \
						 -lz

# 这里递归遍历3级子目录
DIRS = $(shell find $(SRC_DIR) -maxdepth 3 -type d)

# 将每个子目录添加到搜索路径
VPATH = $(DIRS)

SOURCES = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(SOURCES))))
DEPS = $(addprefix $(DEPS_DIR)/, $(patsubst %.cpp, %.d, $(notdir $(SOURCES))))

# 最终目标，可执行文件
$(TARGET): $(OBJS)
	$(CC) $^ $(LNK_FLAGS) -o $@

$(OBJ_DIR)/%.o: %.cpp
	if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi; \
		$(CC) -c $(CC_FLAGS) -o $@ $<

$(DEPS_DIR)/%.d: %.cpp
	@if [ ! -d $(DEPS_DIR) ]; then mkdir -p $(DEPS_DIR); fi; \
		set -e; rm -f $@; \
		$(CC) -MM $(CC_FLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*/$(OBJ_DIR),\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

-include $(DEPS)

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

