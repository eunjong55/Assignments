#include <unistd.h>
#include <stdio.h>
#include "smalloc2.0.h"
#include "string.h"

sm_container_t sm_head = {
	0,
	&sm_head,
	&sm_head,
	0
} ;

size_t totalsize=0;

static
void *
_data (sm_container_ptr e)
{
	return ((void *) e) + sizeof(sm_container_t) ;
}

void
sshrink()
{
		sm_container_ptr itr;
		sm_container_ptr temp;
		itr = sm_head.prev;
		while((itr->prev != &sm_head) && (itr->status == Unused)){
			itr = itr->prev;
		}
		temp = itr->next;
		itr->next = &sm_head;
		sm_head.prev = itr;
		printf("@@@@original break point : %p@@@@ \n", sbrk(0));
		brk(temp);
		printf("@@@@chabged break point : %p@@@@\n", sbrk(0));
}


static
void
sm_container_split (sm_container_ptr hole, size_t size)
{
	sm_container_ptr remainder = (sm_container_ptr) (_data(hole) + size) ;

	remainder->dsize = hole->dsize - size - sizeof(sm_container_t) ;
	remainder->status = Unused ;
	remainder->next = hole->next ;
	remainder->prev = hole ;
	hole->dsize = size ;
	hole->next->prev = remainder ;
	hole->next = remainder ;
}

static
void *
retain_more_memory (int size)
{
	sm_container_ptr hole ;
	int pagesize = getpagesize() ;
	int n_pages = 0 ;

	n_pages = (sizeof(sm_container_t) + size + sizeof(sm_container_t)) / pagesize  + 1 ;
	hole = (sm_container_ptr) sbrk(n_pages * pagesize) ;
	if (hole == 0x0)
		return 0x0 ;
	totalsize+= n_pages * pagesize;
	hole->status = Unused ;
	hole->dsize = n_pages * getpagesize() - sizeof(sm_container_t) ;
	return hole ;
}

void *
smalloc (size_t size)
{
	sm_container_ptr hole = 0x0, itr = 0x0 ;
    	sm_container_ptr min = 0x0 ;

	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next)
	{
		if (itr->status == Busy)
			continue ;

		if ((itr->dsize == size) || (size + sizeof(sm_container_t) < itr->dsize))
		{
            if(min == 0x0 || itr->dsize < min->dsize){
                min = itr;
            }
		}
	}
	hole = min ;
	if (hole == 0x0) {
		hole = retain_more_memory(size) ;
		if (hole == 0x0)
			return 0x0 ;
		hole->next = &sm_head ;
		hole->prev = sm_head.prev ;
		(sm_head.prev)->next = hole ;
		sm_head.prev = hole ;
	}
	if (size < hole->dsize)
		sm_container_split(hole, size) ;
	hole->status = Busy ;
	return _data(hole) ;
}

void
sfree (void * p)
{
	sm_container_ptr itr ;
	sm_container_ptr itr2 ;
	sm_container_ptr itr3 ;
	size_t holesize= 0;
	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (p == _data(itr))
		{
			itr->status = Unused;
			itr2 = itr;
			holesize = 0;
			while((itr2->prev != &sm_head) && (itr2->prev->status == Unused)){
				holesize += itr2->dsize + sizeof(sm_container_t);
				itr2 = itr2->prev;
			}
			itr2->status = Unused;
			itr2->dsize = itr2->dsize+holesize;
			itr2->next = itr->next;
        		itr->next->prev = itr2;

			itr = itr->next;
			itr3 = itr;
			holesize = 0;
			while((itr3 != &sm_head) && (itr3->status == Unused)){
            			holesize += itr3->dsize + sizeof(sm_container_t);
				itr3 = itr3->next;
			}
			itr2->dsize = itr2->dsize + holesize;
                        itr2->next = itr3;
                        itr3->prev = itr2;

			break;
		}
	}
}

void
print_sm_containers ()
{
	sm_container_ptr itr ;
	int i ;

	printf("==================== sm_containers ====================\n") ;
	for (itr = sm_head.next, i = 0 ; itr != &sm_head ; itr = itr->next, i++) {
		printf("%3d:%p:%s:", i, _data(itr), itr->status == Unused ? "Unused" : "  Busy") ;
		printf("%8d:", (int) itr->dsize) ;

		int j ;
		char * s = (char *) _data(itr) ;
		for (j = 0 ; j < (itr->dsize >= 8 ? 8 : itr->dsize) ; j++)
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

}

void
print_mem_uses ()
{
	sm_container_ptr itr ;
	int i ;
	size_t using =  0;
	size_t idle = 0;
	for (itr = sm_head.next, i = 0 ; itr != &sm_head ; itr = itr->next, i++) {
		if(itr->status == Busy){
			using+= itr->dsize;
		}
		else{
			idle+= itr->dsize;
		}
	}
	fprintf(stderr, "the amount of memory retained smalloc so far : %lu \n", totalsize);
  fprintf(stderr, "the amount of memory allocated by smalloc at this moment : %lu \n", using);
	fprintf(stderr, "the amount of memory retained by smalloc but not currently llocated : %lu \n", idle);
}
void *
srealloc (void * p, size_t newsize)
{
        sm_container_ptr itr ;
        sm_container_ptr hole ;
        for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next)
        {
                if (p == _data(itr))
                {
                	if(itr->dsize == newsize)
                        {
                                return p;
                        }
                        else if(itr->dsize < newsize)
                        {
                                if( (itr->next!=&sm_head) && (itr->next->status == Unused) && (itr->next->dsize >= newsize - itr->dsize) )
                                {
																	sm_container_ptr remainder = (sm_container_ptr) (itr->next + (newsize-itr->dsize)) ;

			        										remainder->dsize = itr->next->dsize - (newsize - itr->dsize) ;
			        										remainder->status = Unused ;
			        										remainder->next = itr->next->next ;
				        									remainder->prev = itr ;

																	itr->dsize = newsize;
																	itr->next = remainder;
																	return _data(itr);
                                }
																else
                                {
																		hole = smalloc(newsize);
																		hole = ((void *) hole) - sizeof(sm_container_t);
																		hole->status = itr->status;
																		hole->dsize= newsize;
													          itr->status = Unused;
																		memcpy(_data(hole), _data(itr), itr->dsize);
                                        return _data(hole);
                                }
                        }
                        else if(itr->dsize > newsize)
                        {
                                sm_container_split(itr, newsize) ;
                                return _data(itr);
                        }
                }
        }
}
