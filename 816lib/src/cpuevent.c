/*
 * lib65816/cpuevent.c Release 1p2
 * See LICENSE for more details.
 *
 * Copyright 2007 by Samuel A. Falvo II
 */

#include "lib65816/cpuevent.h"
#include <stdio.h>

struct List
{
    CPUEvent *  head;
    void *      null;
    CPUEvent *  tail;
};

static struct List eventList;

void
CPUEvent_initialize( void )
{
    eventList.head = (CPUEvent *)0;
    eventList.null = 0;
    eventList.tail = (CPUEvent *)0;
}

void
CPUEvent_elapse( word32 cycles )
{
    eventList.head -> counter -= cycles;
    if( eventList.head -> counter < 0 )
    {
        eventList.head -> counter = 0;
        CPUEvent_dispatch();
    }
}

void
CPUEvent_schedule( CPUEvent *thisEvent, word32 when, CPUHandler *proc )
{
    CPUEvent *p, *q;

    thisEvent -> counter = when;
    thisEvent -> handler = proc;

    if(!eventList.head) {

        thisEvent->next = 0;
        thisEvent->previous = 0;
        eventList.head = thisEvent;
        return;
    }

    p = eventList.head;
    q = p -> next;

    while( ( thisEvent -> counter ) && ( q -> next ) )
    {
        /* Newly scheduled event is before 'q', so insert it in front of
         * q and compensate q's countdown accordingly.
         */

        if( thisEvent -> counter < q -> counter )
        {
            p -> next = thisEvent;     thisEvent -> next = q;
            q -> previous = thisEvent; thisEvent -> previous = p;

            q -> counter -= thisEvent -> counter;
            return;
        }
        
        /* Otherwise, q occurs before thisEvent, so we compensate thisEvent's counter
         * as we continue to find the ideal insertion point.
         */

        thisEvent -> counter -= q -> counter;
        if( thisEvent -> counter < 0 ) thisEvent -> counter = 0;
        p = q;
        q = q -> next;
    }

    p -> next = thisEvent;     thisEvent -> next = q;
    q -> previous = thisEvent; thisEvent -> previous = p;
}

void
CPUEvent_dispatch( void )
{
    CPUEvent *p, *nextEvent, *thisEvent;

    thisEvent = eventList.head;
    while( thisEvent -> next )
    {
        if( thisEvent -> counter != 0 ) return;

        /* We need to dequeue the node FIRST, because the called
         * handler may attempt to reschedule the event.
         */

        p = thisEvent -> previous;     nextEvent = thisEvent -> next;
        if(p) p -> next = nextEvent;          
        if(nextEvent) nextEvent -> previous = p;
        if(eventList.head == thisEvent)
            eventList.head = nextEvent;

        thisEvent -> handler( 0 ); thisEvent = nextEvent;
    }
}

