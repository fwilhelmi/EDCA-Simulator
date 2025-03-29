
#line 5 "EdcaSim.cc"
#include <stdio.h>
#include <iostream>


#line 1 "./COST/cost.h"

























#ifndef queue_t
#define queue_t SimpleQueue
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <deque>
#include <vector>
#include <assert.h>


#line 1 "./COST/priority_q.h"























#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include <stdio.h>
#include <string.h>














template < class ITEM >
class SimpleQueue 
{
 public:
  SimpleQueue() :m_head(NULL) {};
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  ITEM* NextEvent() const { return m_head; };
  const char* GetName();
 protected:
  ITEM* m_head;
};

template <class ITEM>
const char* SimpleQueue<ITEM>::GetName()
{
  static const char* name = "SimpleQueue";
  return name;
}

template <class ITEM>
void SimpleQueue<ITEM>::EnQueue(ITEM* item)
{
  if( m_head==NULL || item->time < m_head->time )
  {
    if(m_head!=NULL)m_head->prev=item;
    item->next=m_head;
    m_head=item;
    item->prev=NULL;
    return;
  }
    
  ITEM* i=m_head;
  while( i->next!=NULL && item->time > i->next->time)
    i=i->next;
  item->next=i->next;
  if(i->next!=NULL)i->next->prev=item;
  i->next=item;
  item->prev=i;

}

template <class ITEM>
ITEM* SimpleQueue<ITEM> ::DeQueue()
{
  if(m_head==NULL)return NULL;
  ITEM* item = m_head;
  m_head=m_head->next;
  if(m_head!=NULL)m_head->prev=NULL;
  return item;
}

template <class ITEM>
void SimpleQueue<ITEM>::Delete(ITEM* item)
{
  if(item==NULL) return;

  if(item==m_head)
  {
    m_head=m_head->next;
    if(m_head!=NULL)m_head->prev=NULL;
  }
  else
  {
    item->prev->next=item->next;
    if(item->next!=NULL)
      item->next->prev=item->prev;
  }

}

template <class ITEM>
class GuardedQueue : public SimpleQueue<ITEM>
{
 public:
  void Delete(ITEM*);
  void EnQueue(ITEM*);
  bool Validate(const char*);
};
template <class ITEM>
void GuardedQueue<ITEM>::EnQueue(ITEM* item)
{

  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=NULL)
  {
    if(i==item)
    {
      pthread_printf("queue error: item %f(%p) is already in the queue\n",item->time,item);
    }
    i=i->next;
  }
  SimpleQueue<ITEM>::EnQueue(item);
}

template <class ITEM>
void GuardedQueue<ITEM>::Delete(ITEM* item)
{
  ITEM* i=SimpleQueue<ITEM>::m_head;
  while(i!=item&&i!=NULL)
    i=i->next;
  if(i==NULL)
    pthread_printf("error: cannot find the to-be-deleted event %f(%p)\n",item->time,item);
  else
    SimpleQueue<ITEM>::Delete(item);
}

template <class ITEM>
bool GuardedQueue<ITEM>::Validate(const char* s)
{
  char out[1000],buff[100];

  ITEM* i=SimpleQueue<ITEM>::m_head;
  bool qerror=false;

  sprintf(out,"queue error %s : ",s);
  while(i!=NULL)
  {
    sprintf(buff,"%f ",i->time);
    strcat(out,buff);
    if(i->next!=NULL)
      if(i->next->prev!=i)
      {
	qerror=true;
	sprintf(buff," {broken} ");
	strcat(out,buff);
      }
    if(i==i->next)
    {
      qerror=true;
      sprintf(buff,"{loop}");
      strcat(out,buff);
      break;
    }
    i=i->next;
  }
  if(qerror)
    printf("%s\n",out);
  return qerror;
}

template <class ITEM>
class ErrorQueue : public SimpleQueue<ITEM>
{
 public:
  ITEM* DeQueue(double);
  const char* GetName();
};

template <class ITEM>
const char* ErrorQueue<ITEM>::GetName()
{
  static const char* name = "ErrorQueue";
  return name;
}

template <class ITEM>
ITEM* ErrorQueue<ITEM> ::DeQueue(double stoptime)
{
  

  if(drand48()>0.5)
    return SimpleQueue<ITEM>::DeQueue();

  int s=0;
  ITEM* e;
  e=SimpleQueue<ITEM>::m_head;
  while(e!=NULL&&e->time<stoptime)
  {
    s++;
    e=e->next;
  }
  e=SimpleQueue<ITEM>::m_head;
  s=(int)(s*drand48());
  while(s!=0)
  {
    e=e->next;
    s--;
  }
  Delete(e);
  return e;
}

template < class ITEM >
class HeapQueue 
{
 public:
  HeapQueue();
  ~HeapQueue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  void Delete(ITEM*);
  const char* GetName();
  ITEM* NextEvent() const { return num_of_elems?elems[0]:NULL; };
 private:
  void SiftDown(int);
  void PercolateUp(int);
  void Validate(const char*);
        
  ITEM** elems;
  int num_of_elems;
  int curr_max;
};

template <class ITEM>
const char* HeapQueue<ITEM>::GetName()
{
  static const char* name = "HeapQueue";
  return name;
}

template <class ITEM>
void HeapQueue<ITEM>::Validate(const char* s)
{
  int i,j;
  char out[1000],buff[100];
  for(i=0;i<num_of_elems;i++)
    if(  ((2*i+1)<num_of_elems&&elems[i]->time>elems[2*i+1]->time) ||
	 ((2*i+2)<num_of_elems&&elems[i]->time>elems[2*i+2]->time) )
    {
      sprintf(out,"queue error %s : ",s);
      for(j=0;j<num_of_elems;j++)
      {
	if(i!=j)
	  sprintf(buff,"%f(%d) ",elems[j]->time,j);
	else
	  sprintf(buff,"{%f(%d)} ",elems[j]->time,j);
	strcat(out,buff);
      }
      printf("%s\n",out);
    }
}
template <class ITEM>
HeapQueue<ITEM>::HeapQueue()
{
  curr_max=16;
  elems=new ITEM*[curr_max];
  num_of_elems=0;
}
template <class ITEM>
HeapQueue<ITEM>::~HeapQueue()
{
  delete [] elems;
}
template <class ITEM>
void HeapQueue<ITEM>::SiftDown(int node)
{
  if(num_of_elems<=1) return;
  int i=node,k,c1,c2;
  ITEM* temp;
        
  do{
    k=i;
    c1=c2=2*i+1;
    c2++;
    if(c1<num_of_elems && elems[c1]->time < elems[i]->time)
      i=c1;
    if(c2<num_of_elems && elems[c2]->time < elems[i]->time)
      i=c2;
    if(k!=i)
    {
      temp=elems[i];
      elems[i]=elems[k];
      elems[k]=temp;
      elems[k]->pos=k;
      elems[i]->pos=i;
    }
  }while(k!=i);
}
template <class ITEM>
void HeapQueue<ITEM>::PercolateUp(int node)
{
  int i=node,k,p;
  ITEM* temp;
        
  do{
    k=i;
    if( (p=(i+1)/2) != 0)
    {
      --p;
      if(elems[i]->time < elems[p]->time)
      {
	i=p;
	temp=elems[i];
	elems[i]=elems[k];
	elems[k]=temp;
	elems[k]->pos=k;
	elems[i]->pos=i;
      }
    }
  }while(k!=i);
}

template <class ITEM>
void HeapQueue<ITEM>::EnQueue(ITEM* item)
{
  if(num_of_elems>=curr_max)
  {
    curr_max*=2;
    ITEM** buffer=new ITEM*[curr_max];
    for(int i=0;i<num_of_elems;i++)
      buffer[i]=elems[i];
    delete[] elems;
    elems=buffer;
  }
        
  elems[num_of_elems]=item;
  elems[num_of_elems]->pos=num_of_elems;
  num_of_elems++;
  PercolateUp(num_of_elems-1);
}

template <class ITEM>
ITEM* HeapQueue<ITEM>::DeQueue()
{
  if(num_of_elems<=0)return NULL;
        
  ITEM* item=elems[0];
  num_of_elems--;
  elems[0]=elems[num_of_elems];
  elems[0]->pos=0;
  SiftDown(0);
  return item;
}

template <class ITEM>
void HeapQueue<ITEM>::Delete(ITEM* item)
{
  int i=item->pos;

  num_of_elems--;
  elems[i]=elems[num_of_elems];
  elems[i]->pos=i;
  SiftDown(i);
  PercolateUp(i);
}



#define CQ_MAX_SAMPLES 25

template <class ITEM>
class CalendarQueue 
{
 public:
  CalendarQueue();
  const char* GetName();
  ~CalendarQueue();
  void enqueue(ITEM*);
  ITEM* dequeue();
  void EnQueue(ITEM*);
  ITEM* DeQueue();
  ITEM* NextEvent() const { return m_head;}
  void Delete(ITEM*);
 private:
  long last_bucket,number_of_buckets;
  double bucket_width;
        
  void ReSize(long);
  double NewWidth();

  ITEM ** buckets;
  long total_number;
  double bucket_top;
  long bottom_threshold;
  long top_threshold;
  double last_priority;
  bool resizable;

  ITEM* m_head;
  char m_name[100];
};


template <class ITEM>
const char* CalendarQueue<ITEM> :: GetName()
{
  sprintf(m_name,"Calendar Queue (bucket width: %.2e, size: %ld) ",
	  bucket_width,number_of_buckets);
  return m_name;
}
template <class ITEM>
CalendarQueue<ITEM>::CalendarQueue()
{
  long i;
        
  number_of_buckets=16;
  bucket_width=1.0;
  bucket_top=bucket_width;
  total_number=0;
  last_bucket=0;
  last_priority=0.0;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  resizable=true;
        
  buckets= new ITEM*[number_of_buckets];
  for(i=0;i<number_of_buckets;i++)
    buckets[i]=NULL;
  m_head=NULL;

}
template <class ITEM>
CalendarQueue<ITEM>::~CalendarQueue()
{
  delete [] buckets;
}
template <class ITEM>
void CalendarQueue<ITEM>::ReSize(long newsize)
{
  long i;
  ITEM** old_buckets=buckets;
  long old_number=number_of_buckets;
        
  resizable=false;
  bucket_width=NewWidth();
  buckets= new ITEM*[newsize];
  number_of_buckets=newsize;
  for(i=0;i<newsize;i++)
    buckets[i]=NULL;
  last_bucket=0;
  total_number=0;

  
        
  ITEM *item;
  for(i=0;i<old_number;i++)
  {
    while(old_buckets[i]!=NULL)
    {
      item=old_buckets[i];
      old_buckets[i]=item->next;
      enqueue(item);
    }
  }
  resizable=true;
  delete[] old_buckets;
  number_of_buckets=newsize;
  top_threshold=number_of_buckets*2;
  bottom_threshold=number_of_buckets/2-2;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  last_bucket = long(last_priority/bucket_width) % number_of_buckets;

}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::DeQueue()
{
  ITEM* head=m_head;
  m_head=dequeue();
  return head;
}
template <class ITEM>
ITEM* CalendarQueue<ITEM>::dequeue()
{
  long i;
  for(i=last_bucket;;)
  {
    if(buckets[i]!=NULL&&buckets[i]->time<bucket_top)
    {
      ITEM * item=buckets[i];
      buckets[i]=buckets[i]->next;
      total_number--;
      last_bucket=i;
      last_priority=item->time;
                        
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      item->next=NULL;
      return item;
    }
    else
    {
      i++;
      if(i==number_of_buckets)i=0;
      bucket_top+=bucket_width;
      if(i==last_bucket)
	break;
    }
  }
        
  
  long smallest;
  for(smallest=0;smallest<number_of_buckets;smallest++)
    if(buckets[smallest]!=NULL)break;

  if(smallest >= number_of_buckets)
  {
    last_priority=bucket_top;
    return NULL;
  }

  for(i=smallest+1;i<number_of_buckets;i++)
  {
    if(buckets[i]==NULL)
      continue;
    else
      if(buckets[i]->time<buckets[smallest]->time)
	smallest=i;
  }
  ITEM * item=buckets[smallest];
  buckets[smallest]=buckets[smallest]->next;
  total_number--;
  last_bucket=smallest;
  last_priority=item->time;
  bucket_top=bucket_width*((long)(last_priority/bucket_width)+1)+bucket_width*0.5;
  item->next=NULL;
  return item;
}
template <class ITEM>
void CalendarQueue<ITEM>::EnQueue(ITEM* item)
{
  
  if(m_head==NULL)
  {
    m_head=item;
    return;
  }
  if(m_head->time>item->time)
  {
    enqueue(m_head);
    m_head=item;
  }
  else
    enqueue(item);
}
template <class ITEM>
void CalendarQueue<ITEM>::enqueue(ITEM* item)
{
  long i;
  if(item->time<last_priority)
  {
    i=(long)(item->time/bucket_width);
    last_priority=item->time;
    bucket_top=bucket_width*(i+1)+bucket_width*0.5;
    i=i%number_of_buckets;
    last_bucket=i;
  }
  else
  {
    i=(long)(item->time/bucket_width);
    i=i%number_of_buckets;
  }

        
  

  if(buckets[i]==NULL||item->time<buckets[i]->time)
  {
    item->next=buckets[i];
    buckets[i]=item;
  }
  else
  {

    ITEM* pos=buckets[i];
    while(pos->next!=NULL&&item->time>pos->next->time)
    {
      pos=pos->next;
    }
    item->next=pos->next;
    pos->next=item;
  }
  total_number++;
  if(resizable&&total_number>top_threshold)
    ReSize(number_of_buckets*2);
}
template <class ITEM>
void CalendarQueue<ITEM>::Delete(ITEM* item)
{
  if(item==m_head)
  {
    m_head=dequeue();
    return;
  }
  long j;
  j=(long)(item->time/bucket_width);
  j=j%number_of_buckets;
        
  

  
  

  ITEM** p = &buckets[j];
  
  ITEM* i=buckets[j];
    
  while(i!=NULL)
  {
    if(i==item)
    { 
      (*p)=item->next;
      total_number--;
      if(resizable&&total_number<bottom_threshold)
	ReSize(number_of_buckets/2);
      return;
    }
    p=&(i->next);
    i=i->next;
  }   
}
template <class ITEM>
double CalendarQueue<ITEM>::NewWidth()
{
  long i, nsamples;
        
  if(total_number<2) return 1.0;
  if(total_number<=5)
    nsamples=total_number;
  else
    nsamples=5+total_number/10;
  if(nsamples>CQ_MAX_SAMPLES) nsamples=CQ_MAX_SAMPLES;
        
  long _last_bucket=last_bucket;
  double _bucket_top=bucket_top;
  double _last_priority=last_priority;
        
  double AVG[CQ_MAX_SAMPLES],avg1=0,avg2=0;
  ITEM* list,*next,*item;
        
  list=dequeue(); 
  long real_samples=0;
  while(real_samples<nsamples)
  {
    item=dequeue();
    if(item==NULL)
    {
      item=list;
      while(item!=NULL)
      {
	next=item->next;
	enqueue(item);
	item=next;      
      }

      last_bucket=_last_bucket;
      bucket_top=_bucket_top;
      last_priority=_last_priority;

                        
      return 1.0;
    }
    AVG[real_samples]=item->time-list->time;
    avg1+=AVG[real_samples];
    if(AVG[real_samples]!=0.0)
      real_samples++;
    item->next=list;
    list=item;
  }
  item=list;
  while(item!=NULL)
  {
    next=item->next;
    enqueue(item);
    item=next;      
  }
        
  last_bucket=_last_bucket;
  bucket_top=_bucket_top;
  last_priority=_last_priority;
        
  avg1=avg1/(double)(real_samples-1);
  avg1=avg1*2.0;
        
  
  long count=0;
  for(i=0;i<real_samples-1;i++)
  {
    if(AVG[i]<avg1&&AVG[i]!=0)
    {
      avg2+=AVG[i];
      count++;
    }
  }
  if(count==0||avg2==0)   return 1.0;
        
  avg2 /= (double) count;
  avg2 *= 3.0;
        
  return avg2;
}

#endif /*PRIORITY_QUEUE_H*/

#line 38 "./COST/cost.h"


#line 1 "./COST/corsa_alloc.h"
































#ifndef corsa_allocator_h
#define corsa_allocator_h

#include <typeinfo>
#include <string>

class CorsaAllocator
{
private:
    struct DT{
#ifdef CORSA_DEBUG
	DT* self;
#endif
	DT* next;
    };
public:
    CorsaAllocator(unsigned int );         
    CorsaAllocator(unsigned int, int);     
    ~CorsaAllocator();		
    void *alloc();		
    void free(void*);
    unsigned int datasize() 
    {
#ifdef CORSA_DEBUG
	return m_datasize-sizeof(DT*);
#else
	return m_datasize; 
#endif
    }
    int size() { return m_size; }
    int capacity() { return m_capacity; }			
    
    const char* GetName() { return m_name.c_str(); }
    void SetName( const char* name) { m_name=name; } 

private:
    CorsaAllocator(const CorsaAllocator& ) {}  
    void Setup(unsigned int,int); 
    void InitSegment(int);
  
    unsigned int m_datasize;
    char** m_segments;	          
    int m_segment_number;         
    int m_segment_max;      
    int m_segment_size;	          
				  
    DT* m_free_list; 
    int m_size;
    int m_capacity;

    int m_free_times,m_alloc_times;
    int m_max_allocs;

    std::string m_name;
};
#ifndef CORSA_NODEF
CorsaAllocator::CorsaAllocator(unsigned int datasize)
{
    Setup(datasize,256);	  
}

CorsaAllocator::CorsaAllocator(unsigned int datasize, int segsize)
{
    Setup(datasize,segsize);
}

CorsaAllocator::~CorsaAllocator()
{
    #ifdef CORSA_DEBUG
    printf("%s -- alloc: %d, free: %d, max: %d\n",GetName(),
	   m_alloc_times,m_free_times,m_max_allocs);
    #endif

    for(int i=0;i<m_segment_number;i++)
	delete[] m_segments[i];	   
    delete[] m_segments;			
}

void CorsaAllocator::Setup(unsigned int datasize,int seg_size)
{

    char buffer[50];
    sprintf(buffer,"%s[%d]",typeid(*this).name(),datasize);
    m_name = buffer;

#ifdef CORSA_DEBUG
    datasize+=sizeof(DT*);  
#endif

    if(datasize<sizeof(DT))datasize=sizeof(DT);
    m_datasize=datasize;
    if(seg_size<16)seg_size=16;    
    m_segment_size=seg_size;			
    m_segment_number=1;		   
    m_segment_max=seg_size;	   
    m_segments= new char* [ m_segment_max ] ;   
    m_segments[0]= new char [m_segment_size*m_datasize];  

    m_size=0;
    m_capacity=0;
    InitSegment(0);

    m_free_times=m_alloc_times=m_max_allocs=00;
}

void CorsaAllocator::InitSegment(int s)
{
    char* p=m_segments[s];
    m_free_list=reinterpret_cast<DT*>(p);
    for(int i=0;i<m_segment_size-1;i++,p+=m_datasize)
    {
	reinterpret_cast<DT*>(p)->next=
	    reinterpret_cast<DT*>(p+m_datasize);
    }
    reinterpret_cast<DT*>(p)->next=NULL;
    m_capacity+=m_segment_size;
}

void* CorsaAllocator::alloc()
{
    #ifdef CORSA_DEBUG
    m_alloc_times++;
    if(m_alloc_times-m_free_times>m_max_allocs)
	m_max_allocs=m_alloc_times-m_free_times;
    #endif
    if(m_free_list==NULL)	
    
    {
	int i;
	if(m_segment_number==m_segment_max)	
	
	
	{
	    m_segment_max*=2;		
	    char** buff;
	    buff=new char* [m_segment_max];   
#ifdef CORSA_DEBUG
	    if(buff==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	    for(i=0;i<m_segment_number;i++)
		buff[i]=m_segments[i];	
	    delete [] m_segments;		
	    m_segments=buff;
	}
	m_segment_size*=2;
	m_segments[m_segment_number]=new char[m_segment_size*m_datasize];
#ifdef CORSA_DEBUG
	    if(m_segments[m_segment_number]==NULL)
	    {
		printf("CorsaAllocator runs out of memeory.\n");
		exit(1);
	    }
#endif
	InitSegment(m_segment_number);
	m_segment_number++;
    }

    DT* item=m_free_list;		
    m_free_list=m_free_list->next;
    m_size++;

#ifdef CORSA_DEBUG
    item->self=item;
    char* p=reinterpret_cast<char*>(item);
    p+=sizeof(DT*);
    
    return static_cast<void*>(p);
#else
    return static_cast<void*>(item);
#endif
}

void CorsaAllocator::free(void* data)
{
#ifdef CORSA_DEBUG
    m_free_times++;
    char* p=static_cast<char*>(data);
    p-=sizeof(DT*);
    DT* item=reinterpret_cast<DT*>(p);
    
    if(item!=item->self)
    {
	if(item->self==(DT*)0xabcd1234)
	    printf("%s: packet at %p has already been released\n",GetName(),p+sizeof(DT*)); 
	else
	    printf("%s: %p is probably not a pointer to a packet\n",GetName(),p+sizeof(DT*));
    }
    assert(item==item->self);
    item->self=(DT*)0xabcd1234;
#else
    DT* item=static_cast<DT*>(data);
#endif

    item->next=m_free_list;
    m_free_list=item;
    m_size--;
}
#endif /* CORSA_NODEF */

#endif /* corsa_allocator_h */

#line 39 "./COST/cost.h"


class trigger_t {};
typedef double simtime_t;

#ifdef COST_DEBUG
#define Printf(x) Print x
#else
#define Printf(x)
#endif



class TimerBase;



struct CostEvent
{
  double time;
  CostEvent* next;
  union {
    CostEvent* prev;
    int pos;  
  };
  TimerBase* object;
  int index;
  unsigned char active;
};



class TimerBase
{
 public:
  virtual void activate(CostEvent*) = 0;
  inline virtual ~TimerBase() {}	
};

class TypeII;



class CostSimEng
{
 public:

  class seed_t
      {
       public:
	void operator = (long seed) { srand48(seed); };
      };
  seed_t		Seed;
  CostSimEng()
      : stopTime( 0), clearStatsTime( 0), m_clock( 0.0)
      {
        if( m_instance == NULL)
	  m_instance = this;
        else
	  printf("Error: only one simulation engine can be created\n");
      }
  virtual		~CostSimEng()	{ }
  static CostSimEng	*Instance()
      {
        if(m_instance==NULL)
        {
	  printf("Error: a simulation engine has not been initialized\n");
	  m_instance = new CostSimEng;
        }
        return m_instance;
      }
  CorsaAllocator	*GetAllocator(unsigned int datasize)
      {
    	for(unsigned int i=0;i<m_allocators.size();i++)
    	{
	  if(m_allocators[i]->datasize()==datasize)return m_allocators[i];
    	} 
    	CorsaAllocator* allocator=new CorsaAllocator(datasize);
    	char buffer[25];
    	sprintf(buffer,"EventAllocator[%d]",datasize);
    	allocator->SetName(buffer);
    	m_allocators.push_back(allocator);
    	return allocator;
      }
  void		AddComponent(TypeII*c)
      {
        m_components.push_back(c);
      }
  void		ScheduleEvent(CostEvent*e)
      {
	if( e->time < m_clock)
	  assert(e->time>=m_clock);
        
        m_queue.EnQueue(e);
      }
  void		CancelEvent(CostEvent*e)
      {
        
        m_queue.Delete(e);
      }
  double	Random( double v=1.0)	{ return v*drand48();}
  int		Random( int v)		{ return (int)(v*drand48()); }
  double	Exponential(double mean)	{ return -mean*log(Random());}
  virtual void	Start()		{}
  virtual void	Stop()		{}
  void		Run();
  double	SimTime()	{ return m_clock; } 
  void		StopTime( double t)	{ stopTime = t; }
  double	StopTime() const	{ return stopTime; }
  void		ClearStatsTime( double t)	{ clearStatsTime = t; }
  double	ClearStatsTime() const	{ return clearStatsTime; }
  virtual void	ClearStats()	{}
 private:
  double	stopTime;
  double	clearStatsTime;	
  double	eventRate;
  double	runningTime;
  long		eventsProcessed;
  double	m_clock;
  queue_t<CostEvent>	m_queue;
  std::vector<TypeII*>	m_components;
  static CostSimEng	*m_instance;
  std::vector<CorsaAllocator*>	m_allocators;
};




class TypeII
{
 public: 
  virtual void Start() {};
  virtual void Stop() {};
  inline virtual ~TypeII() {}		
  TypeII()
      {
        m_simeng=CostSimEng::Instance();
        m_simeng->AddComponent(this);
      }

#ifdef COST_DEBUG
  void Print(const bool, const char*, ...);
#endif
    
  double Random(double v=1.0) { return v*drand48();}
  int Random(int v) { return (int)(v*drand48());}
  double Exponential(double mean) { return -mean*log(Random());}
  inline double SimTime() const { return m_simeng->SimTime(); }
  inline double StopTime() const { return m_simeng->StopTime(); }
 private:
  CostSimEng* m_simeng;
}; 

#ifdef COST_DEBUG
void TypeII::Print(const bool flag, const char* format, ...)
{
  if(flag==false) return;
  va_list ap;
  va_start(ap, format);
  printf("[%.10f] ",SimTime());
  vprintf(format,ap);
  va_end(ap);
}
#endif

CostSimEng* CostSimEng::m_instance = NULL;

void CostSimEng::Run()
{
  double	nextTime = (clearStatsTime != 0.0 && clearStatsTime < stopTime) ? clearStatsTime : stopTime;

  m_clock = 0.0;
  eventsProcessed = 0l;
  std::vector<TypeII*>::iterator iter;
      
  struct timeval start_time;    
  gettimeofday( &start_time, NULL);

  Start();

  for( iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Start();

  CostEvent* e=m_queue.DeQueue();
  while( e != NULL)
  {
    if( e->time >= nextTime)
    {
      if( nextTime == stopTime)
	break;
      
      printf( "Clearing statistics @ %f\n", nextTime);
      nextTime = stopTime;
      ClearStats();
    }
    
    assert( e->time >= m_clock);
    m_clock = e->time;
    e->object->activate( e);
    eventsProcessed++;
    e = m_queue.DeQueue();
  }
  m_clock = stopTime;
  for(iter = m_components.begin(); iter != m_components.end(); iter++)
    (*iter)->Stop();
	    
  Stop();

  struct timeval stop_time;    
  gettimeofday(&stop_time,NULL);

  runningTime = stop_time.tv_sec - start_time.tv_sec +
      (stop_time.tv_usec - start_time.tv_usec) / 1000000.0;
  eventRate = eventsProcessed/runningTime;
  
  
  printf("# -------------------------------------------------------------------------\n");	
  printf("# CostSimEng with %s, stopped at %f\n", m_queue.GetName(), stopTime);	
  printf("# %ld events processed in %.3f seconds, event processing rate: %.0f\n",	
  eventsProcessed, runningTime, eventRate);
  
}







#line 8 "EdcaSim.cc"


#line 1 "TrafficSource.h"





#line 39 "TrafficSource.h"
;


#line 51 "TrafficSource.h"
;


#line 56 "TrafficSource.h"
;


#line 85 "TrafficSource.h"
;

#line 9 "EdcaSim.cc"


#line 1 "Queue.h"





#line 1 "FIFO.h"





#line 24 "FIFO.h"
;


#line 29 "FIFO.h"
;


#line 34 "FIFO.h"
;


#line 39 "FIFO.h"
;


#line 44 "FIFO.h"
;


#line 49 "FIFO.h"
;


#line 54 "FIFO.h"
;


#line 59 "FIFO.h"
;

#line 5 "Queue.h"



#line 122 "Queue.h"
;


#line 127 "Queue.h"
;


#line 143 "Queue.h"
;


#line 170 "Queue.h"
;


#line 354 "Queue.h"
;

#line 10 "EdcaSim.cc"


#line 1 "Channel.h"





#line 28 "Channel.h"
;


#line 33 "Channel.h"
;


#line 38 "Channel.h"
;


#line 48 "Channel.h"
;

#line 11 "EdcaSim.cc"


#line 1 "Sink.h"





#line 41 "Sink.h"
;


#line 61 "Sink.h"
;


#line 78 "Sink.h"
;


#line 93 "Sink.h"
;

#line 12 "EdcaSim.cc"


#line 1 "Tools.h"






#define LOGS_ENABLED 0
#define TRACES_ENABLED 0


#define DATA_RATE 86E6


#define NUMBER_OF_QUEUES 3 // VO, VI & BE


#define AC_VO 0
#define AC_VI 1
#define AC_BE 2


#define BUFFER_SIZE 100


#define B_VO 1E6
#define B_VI 10E6
#define B_BE 5E6


#define EL_VO 250
#define EL_VI 1500
#define EL_BE 12000


#define CHANNEL_BUSY 1
#define CHANNEL_IDLE 0


struct Packet
{
	long seq_number;          
	int L;                    
  double generated_time;    
  double access_time;       
	double sent_time;         
	int source_id;            
	int access_category;      
};

#line 13 "EdcaSim.cc"


#line 1 "Logger.h"




#include <fstream>


#line 41 "Logger.h"
;


#line 62 "Logger.h"
;


#line 68 "Logger.h"
;



#line 78 "Logger.h"
;



#line 87 "Logger.h"
;



#line 94 "Logger.h"
;
#line 14 "EdcaSim.cc"


using namespace std;


#line 133 "EdcaSim.cc"
;

#include "compcxx_EdcaSim.h"

#line 5 "Channel.h"
class compcxx_Channel_9 : public compcxx_component, public TypeII
{
	public:
		void Setup();
		void Start();
		void Stop();

	public:

		
		/*inport */void startStopTransmission(int occupy_or_release, int transmitter_id);
    class my_Channel_channelStatusReport_f_t:public compcxx_functor<Channel_channelStatusReport_f_t>{ public:void  operator() (int channel_status, int transmitter_id) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(channel_status,transmitter_id); return (c[0]->*f[0])(channel_status,transmitter_id);};};my_Channel_channelStatusReport_f_t channelStatusReport_f;/*outport void channelStatusReport(int channel_status, int transmitter_id)*/;

    
		int num_queues;
    int channel_status;
    int transmitter_id;

};


#line 7 "Logger.h"
class compcxx_Logger_11 : public compcxx_component, public TypeII
{
	public:
		void Setup();
		void Start();
		void Stop();

	public:
		
		/*inport */void trace(char* input);
		/*inport */void result(char* input);

	private:
		char filenameTraces[100] = "traces.txt";
		char filenameResults[100] = "results.csv";

	public:
		int collectTraces = 0;
		int collectResults = 0;
		int hardcodedLimit = 1000; 
		char myLabels[100]; 

	private:
		FILE *fileTraces;
		FILE *fileResults;

	private:
		bool DoesThisFileExist(const char* fileName);
		int counter = 0;
};

class compcxx_Queue_8;/*template <class T> */
#line 267 "./COST/cost.h"
class compcxx_Timer_4 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_4() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Queue_8* p_compcxx_parent;};

class compcxx_Queue_8;/*template <class T> */
#line 267 "./COST/cost.h"
class compcxx_Timer_5 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_5() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Queue_8* p_compcxx_parent;};


#line 5 "FIFO.h"
class compcxx_FIFO_6 : public compcxx_component, public TypeII
{	
	private:
		deque <Packet> m_queue;

	public:
		Packet GetFirstPacket();
		Packet GetPacketAt(int n);
		void DelFirstPacket();
		void DeletePacketIn(int i);
		void PutPacket(Packet &packet);
		void PutPacketFront(Packet &packet);
		void PutPacketIn(Packet &packet, int);
		int QueueSize();
};

class compcxx_Queue_8;/*template <class T> */
#line 267 "./COST/cost.h"
class compcxx_Timer_3 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_3() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_Queue_8* p_compcxx_parent;};


#line 7 "Queue.h"
class compcxx_Queue_8 : public compcxx_component, public TypeII
{
	public:

		void Setup();
		void Start();
		void Stop();

    void checkTransmission();

	public:

		
		/*inport */void in(Packet &packet);
		class my_Queue_out_f_t:public compcxx_functor<Queue_out_f_t>{ public:void  operator() (Packet &packet) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(packet); return (c[0]->*f[0])(packet);};};my_Queue_out_f_t out_f;/*outport void out(Packet &packet)*/;
		class my_Queue_trace_f_t:public compcxx_functor<Queue_trace_f_t>{ public:void  operator() (char* input) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(input); return (c[0]->*f[0])(input);};};my_Queue_trace_f_t trace_f;/*outport void trace(char* input)*/;

    /*inport */void getChannelStatus(int new_channel_status, int transmitter_id);
    class my_Queue_modifyChannelStatus_f_t:public compcxx_functor<Queue_modifyChannelStatus_f_t>{ public:void  operator() (int occupy_or_release, int transmitter_id) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(occupy_or_release,transmitter_id); return (c[0]->*f[0])(occupy_or_release,transmitter_id);};};my_Queue_modifyChannelStatus_f_t modifyChannelStatus_f;/*outport void modifyChannelStatus(int occupy_or_release, int transmitter_id)*/;

		
		compcxx_Timer_3 /*<trigger_t> */service_time;
		/*inport */inline void endService(trigger_t& t);

    
    compcxx_Timer_4 /*<trigger_t> */aifs_check;
		/*inport */inline void aifsEnd(trigger_t& t);

    
    compcxx_Timer_5 /*<trigger_t> */backoff_counter;
		/*inport */inline void backoffEnd(trigger_t& t);
    
		compcxx_Queue_8 () { 
      service_time.p_compcxx_parent=this /*connect service_time.to_component,*/; 
      aifs_check.p_compcxx_parent=this /*connect aifs_check.to_component,*/; 
      backoff_counter.p_compcxx_parent=this /*connect backoff_counter.to_component,*/; 
    }

	public:
  
    
    
    double aifs;
    
    double max_txop;
    
    int cw;
    
    double backoff;
    
    double rate;    
    
    int channel_status;  
    
    int backoff_started;

    
    int queue_id;
    
		compcxx_FIFO_6 fifo_queue;            
    
		int buffer_size;   
		
		int blocked_packets;
    
		int arrived_packets;
    
    int queue_status;

    

    
    int num_packets_aggregated;
    int total_num_packets_aggregated;
    
    int expected_length;
    
    double timestamp_access_channel;
    
    int data_transmitted;
    
    int data_generated;
    
    int bo_values;
    int times_bo_computed;
    
    double total_channel_occupancy_time;
    int times_channel_occupied;
    
    double timestamp_start_channel_access;
    double total_channel_access_time;
    int times_channel_accessed;

	private:
		char msg[200];

};

class compcxx_Logger_11;
#line 5 "Sink.h"
class compcxx_Sink_10 : public compcxx_component, public TypeII
{
	public:
		void Setup();
		void Start();
		void Stop();

	public:
		
		/*inport */void in(Packet &packet);
		/*outport void trace(char* input)*/;
		/*outport void result(char* input)*/;

	public:

    
    
    
    double *aggregate_e2e_delay;
    
    double *aggregate_queuing_delay;
    
    double *aggregate_transmission_delay;
    
		int *received_packets;
    
		int *aggregate_L; 


	private:
		char msg[100];
public:compcxx_Logger_11* p_compcxx_Logger_11;};

class compcxx_TrafficSource_7;/*template <class T> */
#line 267 "./COST/cost.h"
class compcxx_Timer_2 : public compcxx_component, public TimerBase
{
 public:
  struct event_t : public CostEvent { trigger_t data; };
  

  compcxx_Timer_2() { m_simeng = CostSimEng::Instance(); m_event.active= false; }
  inline void Set(trigger_t const &, double );
  inline void Set(double );
  inline double GetTime() { return m_event.time; }
  inline bool Active() { return m_event.active; }
  inline trigger_t & GetData() { return m_event.data; }
  inline void SetData(trigger_t const &d) { m_event.data = d; }
  void Cancel();
  /*outport void to_component(trigger_t &)*/;
  void activate(CostEvent*);
 private:
  CostSimEng* m_simeng;
  event_t m_event;
public:compcxx_TrafficSource_7* p_compcxx_parent;};


#line 5 "TrafficSource.h"
class compcxx_TrafficSource_7 : public compcxx_component, public TypeII
{
	public:
		void Setup();
		void Start();
		void Stop();

	public:
  
		
		class my_TrafficSource_out_f_t:public compcxx_functor<TrafficSource_out_f_t>{ public:void  operator() (Packet &packet) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(packet); return (c[0]->*f[0])(packet);};};my_TrafficSource_out_f_t out_f;/*outport void out(Packet &packet)*/;
		class my_TrafficSource_trace_f_t:public compcxx_functor<TrafficSource_trace_f_t>{ public:void  operator() (char* input) { for (unsigned int compcxx_i=1;compcxx_i<c.size();compcxx_i++)(c[compcxx_i]->*f[compcxx_i])(input); return (c[0]->*f[0])(input);};};my_TrafficSource_trace_f_t trace_f;/*outport void trace(char* input)*/;

		
		compcxx_Timer_2 /*<trigger_t> */inter_packet_timer;
		/*inport */inline void new_packet(trigger_t& t); 

		compcxx_TrafficSource_7 () { inter_packet_timer.p_compcxx_parent=this /*connect inter_packet_timer.to_component,*/; }

	public:
		long seq_number; 
		double bandwidth;     
		double packet_generation_rate;
		double L;
		int source_id;
		int access_category;

	private:
		char msg[100];
};


#line 18 "EdcaSim.cc"
class compcxx_EdcaSim_12 : public compcxx_component, public CostSimEng
{
	public:
		void Setup();
		void Start();		
		void Stop();

    void SetupVariablesByReadingConfigFile();
    void displayScenarioConfiguration();
		
	public:

    
    int num_sources_vo;
    int num_sources_vi;
    int num_sources_be;
    
    int* cw_array;
    
    double* aifs_array;
    
    double* max_txop_array;

		compcxx_array<compcxx_TrafficSource_7 >source;
		compcxx_array<compcxx_Queue_8 >queue;
    compcxx_array<compcxx_Channel_9 >channel;
		compcxx_Sink_10 sink;
		compcxx_Logger_11 log;

};


#line 25 "Channel.h"
void compcxx_Channel_9 :: Setup()
{
  channel_status=0;
}
#line 30 "Channel.h"
void compcxx_Channel_9 :: Start()
{
	
}
#line 35 "Channel.h"
void compcxx_Channel_9 :: Stop()
{
  
}
#line 40 "Channel.h"
void compcxx_Channel_9 :: startStopTransmission(int occupy_or_release, int transmitter_id)
{
  
	channel_status = occupy_or_release;
  transmitter_id = transmitter_id;
  
  (channelStatusReport_f(channel_status, transmitter_id));

}
#line 38 "Logger.h"
void compcxx_Logger_11 :: Setup()
{
	
}
#line 43 "Logger.h"
void compcxx_Logger_11 :: Start()
{
	if (collectTraces)
	{
		fileTraces = fopen(filenameTraces, "w"); 
	}

	if (collectResults)
	{
		if (DoesThisFileExist(filenameResults) != 1)
		{
			fileResults = fopen(filenameResults, "w");
			fprintf(fileResults,"%s\n", myLabels);
		}
		else
		{
			fileResults = fopen(filenameResults, "a");
		}
	}
}
#line 64 "Logger.h"
void compcxx_Logger_11 :: Stop()
{
	if (collectTraces) fclose(fileTraces);
	if (collectResults) fclose(fileResults);
}
#line 71 "Logger.h"
void compcxx_Logger_11 :: trace(char* input)
{
	if (collectTraces && counter < hardcodedLimit)
	{
		fprintf(fileTraces,"%s\n", input);
		counter++;
	}
}
#line 81 "Logger.h"
void compcxx_Logger_11 :: result(char* input)
{
	if (collectResults)
	{
		fprintf(fileResults,"%s\n", input);
	}
}
#line 90 "Logger.h"
bool compcxx_Logger_11 :: DoesThisFileExist(const char* filename)
{
	std::ifstream infile(filename);
	return infile.good();
}
#line 288 "./COST/cost.h"

#line 288 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 "./COST/cost.h"

#line 300 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 "./COST/cost.h"

#line 311 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 "./COST/cost.h"

#line 319 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_4/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->aifsEnd(m_event.data));
}




#line 288 "./COST/cost.h"

#line 288 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 "./COST/cost.h"

#line 300 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 "./COST/cost.h"

#line 311 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 "./COST/cost.h"

#line 319 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_5/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->backoffEnd(m_event.data));
}




#line 21 "FIFO.h"
Packet compcxx_FIFO_6 :: GetFirstPacket()
{
	return(m_queue.front());	
}
#line 26 "FIFO.h"
Packet compcxx_FIFO_6 :: GetPacketAt(int n)
{
	return(m_queue.at(n));	
}
#line 31 "FIFO.h"
void compcxx_FIFO_6 :: DelFirstPacket()
{
	m_queue.pop_front();
}
#line 36 "FIFO.h"
void compcxx_FIFO_6 :: PutPacket(Packet &packet)
{	
	m_queue.push_back(packet);
}
#line 41 "FIFO.h"
void compcxx_FIFO_6 :: PutPacketFront(Packet &packet)
{	
	m_queue.push_front(packet);
}
#line 46 "FIFO.h"
int compcxx_FIFO_6 :: QueueSize()
{
	return(m_queue.size());
}
#line 51 "FIFO.h"
void compcxx_FIFO_6 :: PutPacketIn(Packet & packet,int i)
{
	m_queue.insert(m_queue.begin()+i,packet);
}
#line 56 "FIFO.h"
void compcxx_FIFO_6 :: DeletePacketIn(int i)
{
	m_queue.erase(m_queue.begin()+i);
}
#line 288 "./COST/cost.h"

#line 288 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 "./COST/cost.h"

#line 300 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 "./COST/cost.h"

#line 311 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 "./COST/cost.h"

#line 319 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_3/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->endService(m_event.data));
}




#line 29 "Queue.h"

#line 33 "Queue.h"

#line 37 "Queue.h"

#line 105 "Queue.h"
void compcxx_Queue_8 :: Setup()
{
    
    blocked_packets = 0;
    arrived_packets = 0;
    data_transmitted = 0;
    data_generated = 0;
    bo_values = 0;
    times_bo_computed = 0;
    total_channel_occupancy_time = 0;
    times_channel_occupied = 0;
    timestamp_start_channel_access = 0;
    total_channel_access_time = 0;
    times_channel_accessed = 0;
    total_num_packets_aggregated = 0;
    backoff_started = 0;
    
}
#line 124 "Queue.h"
void compcxx_Queue_8 :: Start()
{
  
}
#line 129 "Queue.h"
void compcxx_Queue_8 :: Stop()
{

  
	printf("\n###### Results Node N%d ######\n", queue_id);
  printf("      - Traffic generation rate = %f Mbps\n", (double)data_generated*1E-6/SimTime());
  printf("      - Mean Backoff = %f slots\n", (double)bo_values/times_bo_computed);
  printf("      - Mean channel access time = %f ms\n", 1E3*total_channel_access_time/times_channel_accessed);
  printf("      - Mean channel occupancy time = %f ms\n", 1E3*total_channel_occupancy_time/times_channel_occupied);
  printf("      - Mean num. of packets per transmission = %.2f\n", (double)total_num_packets_aggregated/times_channel_accessed);
  printf("      - Blocking Probability = %f\n", (double)blocked_packets/arrived_packets);
	
  printf("\n");

}
#line 145 "Queue.h"
void compcxx_Queue_8 :: in(Packet &packet)
{

  if(LOGS_ENABLED) printf("%f N%d - New packet arrived to the queue (source_id=%d, seq_number=%li, length=%d bits). There were %d packets in the buffer.\n",
    SimTime(),queue_id,packet.source_id,packet.seq_number,packet.L,fifo_queue.QueueSize());
	sprintf(msg,"%f N%d - New packet arrived to the queue (source_id=%d, seq_number=%li, length=%d bits). There were %d packets in the buffer.",
    SimTime(),queue_id,packet.source_id,packet.seq_number,packet.L,fifo_queue.QueueSize());
	if(TRACES_ENABLED) (trace_f(msg));

	++arrived_packets;

  data_generated += packet.L;

  
  if(fifo_queue.QueueSize() < BUFFER_SIZE) {
    
    fifo_queue.PutPacket(packet);
  }
  else {
    ++blocked_packets;
  }

  
  checkTransmission();

}
#line 172 "Queue.h"
void compcxx_Queue_8 :: checkTransmission() {
  
  if(LOGS_ENABLED) printf("%f N%d - Checking if a transmission can be scheduled (queue size = %d).\n",
    SimTime(),queue_id, fifo_queue.QueueSize());
  sprintf(msg,"%f N%d - Checking if a transmission can be scheduled (queue size = %d).",
    SimTime(),queue_id, fifo_queue.QueueSize());
  if(TRACES_ENABLED) (trace_f(msg));

  
  if(fifo_queue.QueueSize() > 0 && queue_status == 0){
    
    
    if(backoff_started == 0) {   
      int backoff_slots = (rand()%(cw+1));
      backoff = 9E-6*backoff_slots;
      bo_values += backoff_slots;
      ++times_bo_computed;
      backoff_started = 1;
      if(LOGS_ENABLED) printf("%f N%d - New backoff computed = %f s (%d)\n", SimTime(), queue_id, backoff, backoff_slots);
      sprintf(msg,"%f N%d - New backoff computed = %f s (%d)", SimTime(), queue_id, backoff, backoff_slots);
      if(TRACES_ENABLED) (trace_f(msg));
        queue_status = 1;
        timestamp_start_channel_access = SimTime(); 
     }    

    if (channel_status == CHANNEL_IDLE) {
      if(LOGS_ENABLED) printf("%f N%d - A new transmission can be scheduled. Start AIFS...\n", SimTime(), queue_id);
      sprintf(msg,"%f N%d - A new transmission can be scheduled. Start AIFS...", SimTime(), queue_id);
      if(TRACES_ENABLED) (trace_f(msg));
      aifs_check.Set(SimTime() + aifs);
    }

  } 
  
  
  
  
  
  
  
}


#line 214 "Queue.h"
void compcxx_Queue_8 :: getChannelStatus(int new_channel_status, int transmitter_id){
  
  if(LOGS_ENABLED) printf("%f N%d Channel status changed to %d\n", SimTime(), queue_id, new_channel_status);
  sprintf(msg,"%f N%d Channel status changed to %d", SimTime(), queue_id, new_channel_status);
  if(TRACES_ENABLED) (trace_f(msg));

  
  channel_status = new_channel_status;

  
  if(channel_status == CHANNEL_BUSY) {
      if (transmitter_id == queue_id) {
        
      } else {
        
        if (aifs_check.Active()) {
          aifs_check.Cancel();
          if(LOGS_ENABLED) printf("%f N%d - Cancel AIFS\n", SimTime(), queue_id);
          sprintf(msg,"%f N%d - Cancel AIFS", SimTime(), queue_id);
          if(TRACES_ENABLED) (trace_f(msg));
        }
        if (backoff_counter.Active()) {
          backoff = backoff_counter.GetTime() - SimTime();
          backoff_counter.Cancel();
          if(LOGS_ENABLED) printf("%f N%d - Backoff paused at %f s (%f slots)\n", SimTime(), queue_id, backoff, ceil(backoff*9e-6));
          sprintf(msg,"%f N%d - Backoff paused at %f s (%f slots)", SimTime(), queue_id, backoff, ceil(backoff*9e-6));
          if(TRACES_ENABLED) (trace_f(msg));
        }
        
        queue_status = 0;
      }
  } else if (channel_status == CHANNEL_IDLE) {
    
    checkTransmission();
  }

}


#line 252 "Queue.h"
void compcxx_Queue_8 :: aifsEnd(trigger_t &) 
{
  if(LOGS_ENABLED) printf("%f N%d - AIFS ended. Resuming the backof from %f (%d slots)\n", SimTime(), queue_id, backoff, int(backoff*9e-6));
  sprintf(msg,"%f N%d - AIFS ended. Resuming the backof from %f (%d slots)", SimTime(), queue_id, backoff, int(backoff*9e-6));
  if(TRACES_ENABLED) (trace_f(msg));
  
  backoff_counter.Set(SimTime() + backoff);
}


#line 261 "Queue.h"
void compcxx_Queue_8 :: backoffEnd(trigger_t &) 
{

  if(LOGS_ENABLED) printf("%f N%d - Backoff ended.\n", SimTime(), queue_id);
  sprintf(msg,"%f N%d - Backoff ended.", SimTime(), queue_id);
  if(TRACES_ENABLED) (trace_f(msg));
    
  
  (modifyChannelStatus_f(CHANNEL_BUSY, queue_id));

  
  num_packets_aggregated = 0;  
  for (int i = fifo_queue.QueueSize(); i > 0; --i) {  
    
    expected_length = 0; 
    for (int j = 0; j < i; ++j) {
      Packet packet = fifo_queue.GetPacketAt(j);
      expected_length += packet.L;
    }
    
    if (expected_length/rate < max_txop) {
      num_packets_aggregated = i;
      break;
    }
  }
  
  
  timestamp_access_channel = SimTime();
  total_channel_access_time = timestamp_access_channel - timestamp_start_channel_access;
  total_num_packets_aggregated += num_packets_aggregated;
  ++times_channel_accessed;

  
  if(LOGS_ENABLED) printf("%f N%d - Transmitting %d packets, to be finished at %f\n", 
    SimTime(), queue_id, num_packets_aggregated, SimTime()+(expected_length/rate));
  sprintf(msg,"%f N%d - Transmitting %d packets, to be finished at %f", 
    SimTime(), queue_id, num_packets_aggregated, SimTime()+(expected_length/rate));
  if(TRACES_ENABLED) (trace_f(msg));
  service_time.Set(SimTime()+(expected_length/rate));

}


#line 303 "Queue.h"
void compcxx_Queue_8 :: endService(trigger_t &)
{

  if(LOGS_ENABLED) printf("%f N%d - Transmission finished. List of transmitted packets:\n", SimTime(), queue_id);
  sprintf(msg,"%f N%d - Transmission finished. List of transmitted packets:", SimTime(), queue_id);
  if(TRACES_ENABLED) (trace_f(msg));
  
  for (int i = 0; i < num_packets_aggregated; ++i) {

    
    Packet packet = fifo_queue.GetFirstPacket();

    if(LOGS_ENABLED) printf("%f     * seq_number = %li (from source %d), length = %d bits\n", 
      SimTime(), packet.seq_number, packet.source_id, packet.L);
    sprintf(msg,"%f     * seq_number = %li (from source %d), length = %d bits", 
      SimTime(), packet.seq_number, packet.source_id, packet.L);
    if(TRACES_ENABLED) (trace_f(msg));

    
    fifo_queue.DelFirstPacket();

    
    packet.access_time = timestamp_access_channel;

    
    packet.sent_time = SimTime();

    
    data_transmitted += packet.L;

    
    total_channel_occupancy_time += SimTime() - timestamp_access_channel;
    ++times_channel_occupied;

    
    (out_f(packet));
  }

  if(LOGS_ENABLED) printf("%f N%d - Updated queue size = %d\n", SimTime(),queue_id, fifo_queue.QueueSize());
  sprintf(msg,"%f N%d - Updated queue size = %d", SimTime(),queue_id, fifo_queue.QueueSize());
  if(TRACES_ENABLED) (trace_f(msg));

  
  backoff_started = 0;

  
  queue_status = 0;

  
  (modifyChannelStatus_f(CHANNEL_IDLE, queue_id));
	
}
#line 38 "Sink.h"
void compcxx_Sink_10 :: Setup()
{

}
#line 43 "Sink.h"
void compcxx_Sink_10 :: Start()
{

  
  
  aggregate_e2e_delay = new double[NUMBER_OF_QUEUES];
  aggregate_queuing_delay = new double[NUMBER_OF_QUEUES];
  aggregate_transmission_delay = new double[NUMBER_OF_QUEUES];
  received_packets = new int[NUMBER_OF_QUEUES];
  aggregate_L = new int[NUMBER_OF_QUEUES];
  for (int i = 0; i < NUMBER_OF_QUEUES; ++i) {
    aggregate_e2e_delay[i] = 0;
    aggregate_queuing_delay[i] = 0;
    aggregate_transmission_delay[i] = 0;
    received_packets[i] = 0;
    aggregate_L[i] = 0;
  }

}
#line 63 "Sink.h"
void compcxx_Sink_10 :: Stop()
{

	printf("###### SINK: Results per Access Category ######\n");
  for(int i = 0; i < NUMBER_OF_QUEUES; ++i) {
    printf(" * AC%d:\n", i);
    printf("     - Number of packets received = %d\n", received_packets[i]);
    printf("     - Total bits received = %d\n", aggregate_L[i]);
    printf("     - Throughput = %.4f Mbps\n", 1E-6*aggregate_L[i]/SimTime());
    printf("     - Average end-to-end delay = %.2f ms\n", 1E3*aggregate_e2e_delay[i]/received_packets[i]);
    printf("     - Average queuing delay = %.2f ms\n", 1E3*aggregate_queuing_delay[i]/received_packets[i]);
    printf("     - Average transmission delay = %.4f ms\n", 1E3*aggregate_transmission_delay[i]/received_packets[i]);
  }
  printf("\n");

}
#line 80 "Sink.h"
void compcxx_Sink_10 :: in(Packet &packet)
{

	sprintf(msg, "%f - Sink: Packet %li from source %d (AC = %d) received!",
    SimTime(), packet.seq_number, packet.source_id, packet.access_category);
	if(TRACES_ENABLED) (p_compcxx_Logger_11->trace(msg));

	aggregate_e2e_delay[packet.access_category] += SimTime() - packet.generated_time;
  aggregate_queuing_delay[packet.access_category] += packet.access_time - packet.generated_time;
  aggregate_transmission_delay[packet.access_category] += SimTime() - packet.access_time;
  aggregate_L[packet.access_category] += packet.L;
  ++received_packets[packet.access_category];

}
#line 288 "./COST/cost.h"

#line 288 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(trigger_t const & data, double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.data = data;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 300 "./COST/cost.h"

#line 300 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Set(double time)
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.time = time;
  m_event.object = this;
  m_event.active=true;
  m_simeng->ScheduleEvent(&m_event);
}


#line 311 "./COST/cost.h"

#line 311 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::Cancel()
{
  if(m_event.active)
    m_simeng->CancelEvent(&m_event);
  m_event.active = false;
}


#line 319 "./COST/cost.h"

#line 319 "./COST/cost.h"
/*template <class T>
*/void compcxx_Timer_2/*<trigger_t >*/::activate(CostEvent*e)
{
  assert(e==&m_event);
  m_event.active=false;
  (p_compcxx_parent->new_packet(m_event.data));
}




#line 20 "TrafficSource.h"

#line 36 "TrafficSource.h"
void compcxx_TrafficSource_7 :: Setup()
{	

}
#line 41 "TrafficSource.h"
void compcxx_TrafficSource_7 :: Start()
{

  
	seq_number = 0;
	packet_generation_rate = bandwidth/L;

  
  inter_packet_timer.Set(Exponential(1/packet_generation_rate));

}
#line 53 "TrafficSource.h"
void compcxx_TrafficSource_7 :: Stop()
{
	
}
#line 58 "TrafficSource.h"
void compcxx_TrafficSource_7 :: new_packet(trigger_t &)
{
	Packet packet;

	
  packet.L = (int) Exponential(L);

  
  packet.generated_time = SimTime();

	packet.seq_number = seq_number;
	seq_number++;
	
	packet.source_id = source_id;
	packet.access_category = access_category;
	
  if(LOGS_ENABLED) printf("%f S%d - New packet generated (seq_number=%li) of length %d bits\n",
    SimTime(),source_id, packet.seq_number, packet.L);
	sprintf(msg,"%f S%d - New packet generated (seq_number=%li) of length %d bits",
    SimTime(),source_id, packet.seq_number, packet.L);
	if(TRACES_ENABLED) (trace_f(msg));
	
	(out_f(packet));
	
	
  inter_packet_timer.Set(SimTime()+Exponential(1/packet_generation_rate));

}
#line 49 "EdcaSim.cc"
void compcxx_EdcaSim_12 :: Setup()
{

  
  cw_array = new int[NUMBER_OF_QUEUES];
  aifs_array = new double[NUMBER_OF_QUEUES];
  max_txop_array = new double[NUMBER_OF_QUEUES];

  
  SetupVariablesByReadingConfigFile();

  
	log.collectTraces = 1;  
	log.collectResults = 0; 
	sprintf(log.myLabels,"TS1_DELAY,TS2_DELAY,TS1_THROUGHPUT,TS2_THROUGHPUT"); 
	
  int TOTAL_NUM_SOURCES = num_sources_vo+num_sources_vi+num_sources_be;

  
	source.SetSize(TOTAL_NUM_SOURCES);
	
  for (int i = 0; i < num_sources_vo; ++i){
    source[i].source_id = i+1;
    source[i].access_category = AC_VO;
    source[i].L = EL_VO;          
    source[i].bandwidth = B_VO;   
  }
  
  for (int i = num_sources_vo; i < num_sources_vo+num_sources_vi; ++i){
    source[i].source_id = i+1;
    source[i].access_category = AC_VI;
    source[i].L = EL_VI;         
    source[i].bandwidth = B_VI;     
  }
  
  for (int i = num_sources_vo+num_sources_vi; i < num_sources_vo+num_sources_vi+num_sources_be; ++i){
    source[i].source_id = i+1;
    source[i].access_category = AC_BE;
    source[i].L = EL_BE;         
    source[i].bandwidth = B_BE;         
  }

  
  queue.SetSize(NUMBER_OF_QUEUES);
  for (int i = 0; i < NUMBER_OF_QUEUES; ++i){
    
    queue[i].queue_id = i;
    queue[i].cw = cw_array[i];
    queue[i].aifs = aifs_array[i];
    queue[i].max_txop = max_txop_array[i];
    queue[i].rate = DATA_RATE;
    queue[i].buffer_size = BUFFER_SIZE;
  }

	
  channel.SetSize(1);
  channel[0].num_queues = NUMBER_OF_QUEUES;
  
  

  
  for (int i = 0; i < TOTAL_NUM_SOURCES; ++i) {
      source[i].out_f.Connect(queue[source[i].access_category],(compcxx_component::TrafficSource_out_f_t)&compcxx_Queue_8::in) /*connect source[i].out,queue[source[i].access_category].in*/; 
      source[i].trace_f.Connect(log,(compcxx_component::TrafficSource_trace_f_t)&compcxx_Logger_11::trace) /*connect source[i].trace,log.trace*/; 
  }

  
  for (int i = 0; i < NUMBER_OF_QUEUES; ++i) {
      
      queue[i].out_f.Connect(sink,(compcxx_component::Queue_out_f_t)&compcxx_Sink_10::in) /*connect queue[i].out,sink.in*/;      
      
      queue[i].modifyChannelStatus_f.Connect(channel[0],(compcxx_component::Queue_modifyChannelStatus_f_t)&compcxx_Channel_9::startStopTransmission) /*connect queue[i].modifyChannelStatus,channel[0].startStopTransmission*/; 
      channel[0].channelStatusReport_f.Connect(queue[i],(compcxx_component::Channel_channelStatusReport_f_t)&compcxx_Queue_8::getChannelStatus) /*connect channel[0].channelStatusReport,queue[i].getChannelStatus*/; 
      
	    queue[i].trace_f.Connect(log,(compcxx_component::Queue_trace_f_t)&compcxx_Logger_11::trace) /*connect queue[i].trace,log.trace*/; 
  }

  
	sink.p_compcxx_Logger_11=&log /*connect sink.trace,log.trace*/;   
  sink.p_compcxx_Logger_11=&log /*connect sink.result,log.result*/; 

  
  displayScenarioConfiguration();
	
}
#line 135 "EdcaSim.cc"
void compcxx_EdcaSim_12 :: Start()
{
	
}


#line 140 "EdcaSim.cc"
void compcxx_EdcaSim_12 :: Stop()
{
	
}





#line 148 "EdcaSim.cc"
void compcxx_EdcaSim_12 :: SetupVariablesByReadingConfigFile() {

	const char *config_filename = "config_edca";
	char delim[] = "=";
	char *ptr;
	int ix_param = 0;
	printf("\nReading system configuration file '%s'...\n", config_filename);
	FILE* test_input_config = fopen(config_filename, "r");
	if (!test_input_config){
		printf("Config file '%s' not found!\n", config_filename);
		exit(-1);
	}
	char line_system[100];
	while (fgets(line_system, 100, test_input_config)){
		
		if(line_system[0] == '#') {
			continue;
		}
		
		ptr = strtok(line_system, delim);
		ptr = strtok(NULL, delim);
		
		if (ix_param == 0){
			
			num_sources_vo = atoi(ptr);
		} else if (ix_param == 1) {
			
			num_sources_vi = atoi(ptr);
		} else if (ix_param == 2) {
			
			num_sources_be = atoi(ptr);
		} else if (ix_param == 3) {
			
			cw_array[0] = atoi(ptr);
		} else if (ix_param == 4) {
			
			cw_array[1] = atoi(ptr);
		} else if (ix_param == 5) {
			
			cw_array[2] = atoi(ptr);
		} else if (ix_param == 6) {
			
			aifs_array[0] = atof(ptr);
		} else if (ix_param == 7) {
			
			aifs_array[1] = atof(ptr);
		} else if (ix_param == 8) {
			
			aifs_array[2] = atof(ptr);
		} else if (ix_param == 9) {
			
			max_txop_array[0] = atof(ptr);
		} else if (ix_param == 10) {
			
			max_txop_array[1] = atof(ptr);
		} else if (ix_param == 11) {
			
			max_txop_array[2] = atof(ptr);
		}
		++ix_param;
	}
	fclose(test_input_config);

	printf("The simulation scenario was properly set!\n");

}


#line 215 "EdcaSim.cc"
void compcxx_EdcaSim_12 :: displayScenarioConfiguration() {

  printf("-------------------------------\n");
  printf("        SCENARIO DETAILS       \n");
  printf("-------------------------------\n");
  printf("AC_VO (%d):\n", AC_VO);
  printf("  - Number of traffic sources: %d\n", num_sources_vo);
  printf("  - Bandwidth per source: %f Mbps\n", B_VO / 1E6);
  printf("  - Expected packet length: %d bits\n", EL_VO);
  printf("  - CW: %d\n", cw_array[0]);
  printf("  - AIFS: %.2f micro-seconds\n", aifs_array[0] * 1E6);
  printf("  - Max. TXOP duration: %.2f ms\n", max_txop_array[0] * 1E3);
  printf("AC_VI (%d):\n", AC_VI);
  printf("  - Number of traffic sources: %d\n", num_sources_vi);
  printf("  - Bandwidth per source: %f Mbps\n", B_VI / 1E6);
  printf("  - Expected packet length: %d bits\n", EL_VI);
  printf("  - CW: %d\n", cw_array[1]);
  printf("  - AIFS: %.2f micro-seconds\n", aifs_array[1] * 1E6);
  printf("  - Max. TXOP duration: %.2f ms\n", max_txop_array[1] * 1E3);
  printf("AC_BE (%d):\n", AC_BE);
  printf("  - Number of traffic sources: %d\n", num_sources_be);
  printf("  - Bandwidth per source: %f Mbps\n", B_BE / 1E6);
  printf("  - Expected packet length: %d bits\n", EL_BE);
  printf("  - CW: %d\n", cw_array[2]);
  printf("  - AIFS: %.2f micro-seconds\n", aifs_array[2] * 1E6);
  printf("  - Max. TXOP duration: %.2f ms\n", max_txop_array[2] * 1E3);
  printf("-------------------------------\n");

}


#line 245 "EdcaSim.cc"
int main(int argc, char *argv[])
{
  
  long int seed = atof(argv[1]);   
  double sim_time = atof(argv[2]);   

	compcxx_EdcaSim_12 Simulator;
	Simulator.Seed = seed;
	Simulator.StopTime(sim_time);
	Simulator.Setup();
	Simulator.Run();

	return 0;
};
