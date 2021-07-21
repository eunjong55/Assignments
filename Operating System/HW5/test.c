#include <stdio.h>
#include "smalloc2.0.h"

int
main()
{
	void *p1, *p2, *p3, *p4, *p5, *p6;

	print_sm_containers() ;

	p1 = smalloc(2000) ;
	printf("smalloc(2000):%p\n", p1) ;
	print_sm_containers() ;

	p2 = smalloc(800);
	printf("smalloc(800):%p\n", p2) ;
        print_sm_containers() ;

	sfree(p1) ;
        printf("sfree(p1)\n") ;
        print_sm_containers() ;

	p3 = smalloc(1000);
        printf("smalloc(1000):%p\n", p3) ;
        print_sm_containers() ;

	p4 = smalloc(1800) ;
        printf("smalloc(1800):%p\n", p4) ;
        print_sm_containers() ;

	print_mem_uses();
}
