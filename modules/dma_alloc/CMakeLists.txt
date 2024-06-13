
project(dma_alloc)

add_library(${PROJECT_NAME} STATIC
        dma_alloc.h
        dma_alloc.cpp
        dma_buffer.h
)