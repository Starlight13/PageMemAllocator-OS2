#include <iostream>
#include <cmath>
using namespace std;

#define NUM_OF_PAGES 7
#define SINGLE_PAGE_SIZE 256
#define MAX_BLOCKS_ON_PAGE 16

enum class State{
    FREE,
    BLOCK_DIVIDED,
    MULTIPLE_PAGE_BLOCK
};

struct Page{
    State state;
    uint8_t* page_start;
    uint16_t page_class;
    uint8_t list_of_using[MAX_BLOCKS_ON_PAGE];
};

class Allocator {
public:
    
    void init();
    void mem_dump();
    void mem_free(void* adr);
    void* mem_realloc(void* adr, uint16_t size);
    void* mem_alloc(uint16_t size);
private:
    uint8_t* start;
    Page* pages;
    uint16_t align(uint16_t size);
    int find_free_pages_for_multi_block(uint16_t size);
    void unite_page(uint16_t page_index);
};

void Allocator::init() {
    this->pages = new Page[NUM_OF_PAGES];
    this->start = (uint8_t*) malloc(SINGLE_PAGE_SIZE*NUM_OF_PAGES);
    for (int i = 0; i < NUM_OF_PAGES; i++) {
        this->pages[i].state = State::FREE;
        this->pages[i].page_start = this->start + (SINGLE_PAGE_SIZE * i);
        this->pages[i].page_class = SINGLE_PAGE_SIZE;
        for (int j = 0; j < MAX_BLOCKS_ON_PAGE; j++) {
            this->pages[i].list_of_using[j] = 0;
        }
    }
}

void* Allocator::mem_alloc(uint16_t size) {
    if (align(size) > (SINGLE_PAGE_SIZE / 2)){
        int start_index = find_free_pages_for_multi_block(size);
        if (start_index != -1){
            uint16_t pages_needed = align(size) / SINGLE_PAGE_SIZE;
            for (int i = start_index; i < start_index + pages_needed; i++) {
                this->pages[i].state = State::MULTIPLE_PAGE_BLOCK;
                this->pages[i].page_class = align(size);
            }
            return pages[start_index].page_start;
        } else return NULL;
    }
    else {
        int free_page = -1;
        for (int i = 0; i < NUM_OF_PAGES; i++) {
            if(this->pages[i].state == State::FREE && free_page == -1)
                free_page = i;
            if (this->pages[i].state == State::BLOCK_DIVIDED && this->pages[i].page_class == align(size)) {
                for (int j = 0; j < SINGLE_PAGE_SIZE / pages[i].page_class; j++) {
                    if (pages[i].list_of_using[j] == 0) {
                        pages[i].list_of_using[j] = 1;
                        return pages[i].page_start + (pages[i].page_class * j);
                    }
                }
            }
        }
        if(free_page != -1){
                this->pages[free_page].page_class = align(size);
                pages[free_page].list_of_using[0] = 1;
                pages[free_page].state = State::BLOCK_DIVIDED;
                return pages[free_page].page_start;
            }
        }
    return NULL;
}

void* Allocator::mem_realloc(void *adr, uint16_t size) {
    if (adr == NULL) {
        return mem_alloc(size);
    } else {
        for (int i = 0; i < NUM_OF_PAGES; i++) {
            if (pages[i].page_start <= adr && (pages[i].page_start + SINGLE_PAGE_SIZE - 1) > adr) {
                if (align(size) == pages[i].page_class) return adr;
                else {
                    uint16_t block_size = pages[i].page_class;
                    for (int j = 0; j < SINGLE_PAGE_SIZE / pages[i].page_class; j++) {
                        if ((uint8_t **) (pages[i].page_start + (pages[i].page_class * j)) == adr) {
                            mem_free(adr);
                            void* new_adr = mem_alloc(size);
                            if (new_adr != NULL)
                                return new_adr;
                            else {
                                pages[i].page_class = block_size;
                                pages[i].list_of_using[j] = 1;
                                return adr;
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

void Allocator::mem_free(void *adr) {
    for (int i = 0; i < NUM_OF_PAGES; i++) {
        if (((uint8_t*) adr) >= this->pages[i].page_start && ((uint8_t*) adr) <= this->pages[i].page_start + (SINGLE_PAGE_SIZE - 1)){
            if (pages[i].state == State::BLOCK_DIVIDED){
                for (int j = 0; j < SINGLE_PAGE_SIZE / pages[i].page_class; j++) {
                    if ((uint8_t**)(pages[i].page_start + (pages[i].page_class * j)) == adr){
                        pages[i].list_of_using[j] = 0;
                        unite_page(i);
                        return;
                    }
                }
            } else {
                for (int j = 0; j < pages[i].page_class / SINGLE_PAGE_SIZE; j++) {
                    pages[i + j].page_class = SINGLE_PAGE_SIZE;
                    pages[i + j].state = State::FREE;
                }
            }
            break;
        }
    }
}

void Allocator::unite_page(uint16_t page_index) {
    for (int i = 0; i < SINGLE_PAGE_SIZE / pages[page_index].page_class; i++) {
        if (pages[page_index].list_of_using[i] == 1) return;
        if (i == (SINGLE_PAGE_SIZE / pages[page_index].page_class) - 1){
            pages[page_index].state = State::FREE;
            pages[page_index].page_class = SINGLE_PAGE_SIZE;
        }
    }
}

int Allocator::find_free_pages_for_multi_block(uint16_t size) {
    uint16_t needed_pages = align(size) / SINGLE_PAGE_SIZE;
    for (int i = 0; i < NUM_OF_PAGES; i++){
        if (this->pages[i].state == State::FREE && (i + needed_pages) <= NUM_OF_PAGES) {
            if (needed_pages == 1) return i;
            for (int j = 1; j < needed_pages; j++) {
                if (this->pages[i + j].state != State::FREE) {
                    break;
                } else {
                    if ((needed_pages - 1) == j) {
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}

void Allocator::mem_dump() {
    cout << "==Allocator==" << endl;
    for (int i = 0; i < NUM_OF_PAGES; i++) {
        cout << "Page N" << (i + 1) << ", Size: " << SINGLE_PAGE_SIZE << endl;
        cout << "Start: " << (uint8_t**)pages[i].page_start << endl;
        cout << "State: " << (pages[i].state == State::FREE ? "FREE" : pages[i].state == State::BLOCK_DIVIDED ? "Block divided" : "Multy-paged") << endl;
        if (pages[i].state == State::BLOCK_DIVIDED) {
            cout << "Class size: " << pages[i].page_class << endl;
            for (int j = 0; j < SINGLE_PAGE_SIZE / pages[i].page_class; j++) {
                cout << "Block N" << (j + 1) << ", is used: " << (bool)pages[i].list_of_using[j] << endl;
            }
        }
        if (pages[i].state == State::MULTIPLE_PAGE_BLOCK) {
            cout << "Class size: " << pages[i].page_class << endl;
        }
        cout << endl;
    }
}

uint16_t Allocator::align(uint16_t size) {
    uint16_t a = 0;
    while (true){
        if (size <= pow(2,(4 + a))) return pow(2,(4 + a));
        else a++;
    }
}

int main() {
    Allocator allocator;
    allocator.init();
    void * adr1 = allocator.mem_alloc(58);
    void * adr2 = allocator.mem_alloc(31);
    void * adr3 = allocator.mem_alloc(61);
    allocator.mem_realloc(adr1, 11);
    allocator.mem_dump();
    return 0;
}
