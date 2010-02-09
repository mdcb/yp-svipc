


///////////////////////////////////////////
///////////////////////////////////////////
//                                       //
//   #####   #     #  #     #  #######   //
//  #     #  #     #  ##   ##  #     #   //
//  #        #     #  # # # #  #     #   //
//   #####   #######  #  #  #  #     #   //
//        #  #     #  #     #  #     #   //
//  #     #  #     #  #     #  #     #   //
//   #####   #     #  #     #  #######   //
//                                       //
///////////////////////////////////////////
///////////////////////////////////////////



//--------------------------------------------------------------------
// shmo_object
//--------------------------------------------------------------------

typedef struct {
   int shmid;
   char desc[SLOT_DESC_STRING_MAX];
   Array *val;
} shmo_object;

static void shmo_obj_free(void *);
static void shmo_obj_print(void *);
static void shmo_obj_eval(void *, int);
static void shmo_obj_extract(void *, long);

static y_userobj_t shmo_class = {
   "shared memory object", shmo_obj_free, shmo_obj_print, shmo_obj_eval, shmo_obj_extract, NULL
   };

struct _tag {
   void (*push)(shmo_object *, int);
   char *name;
   int   oo;            // offsetof
   long  index;         // in globTab
   };

#define PUSH_TAG(name, type_t, pusher)             \
static void name(shmo_object *obj, int oo) {   \
   type_t value = *(type_t*)((char*)obj+oo);       \
   pusher(value);                                  \
   }
PUSH_TAG(push_tag_int,    int,    ypush_long)
PUSH_TAG(push_tag_float,  float,  ypush_double)
PUSH_TAG(push_tag_double, double, ypush_double)
#undef PUSH_TAG

static void push_tag_data(shmo_object *obj, int oo) {
   if (debug) printf ("push_tag_data\n");
   void *res;
//    switch (obj->dataType) {
//       case BYTE:
//          res = (void*) ypush_c(obj->dims);
//          break;
//       case SHORT:
//          res = (void*) ypush_s(obj->dims);
//          break;
//       case USHORT: // promote to int
//       case INT:
//          res = (void*) ypush_i(obj->dims);
//          break;
//       case FLOAT:
//          res = (void*) ypush_f(obj->dims);
//          break;
//       case DOUBLE:
//          res = (void*) ypush_d(obj->dims);
//          break;
//       default:
//          fprintf(stderr,"unsupported dataType %d\n",obj->dataType);
//          ypush_nil();
//          return;
//       }
//    
//    if (obj->dataType!=USHORT) {
//       memcpy(res,obj->data,obj->dims[1]*obj->dims[2]*abs(obj->dataType)/8);
//       }
//    else {
//       int *p=(int*)res;
//       unsigned short *v=(unsigned short*)obj->data;
//       for (int i=0; i<obj->dims[1]*obj->dims[2]; i++) {
//          *p++=*v++;
//          }
//       }
   }

// static void push_tag_dims(shmo_object *obj, int oo) {
//    if (debug) printf ("push_tag_dims\n");
//    long dd[]={1,2};
//    long *res = ypush_l(dd);
//    memcpy(res,&obj->dims[1],3*sizeof(long));
//    }

// our 'methods'
static struct _tag rtdframe_tagtable[] = {
//   {push_tag_val,    "val",      offsetof(shmo_object,val),      -1},
   {0, 0, 0}, // sentinel
   };

static void push_tag(shmo_object *self, long index) {
   struct _tag *entry;

   if (debug) printf ("push_tag\n");

   for (entry = rtdframe_tagtable ; entry->name ; ++entry) {
      if (entry->index == index) {
         entry->push(self, entry->oo);
         return;
         }
      }
   // y_error("invalid tag");
   // fixme --- need to undertand the y_error mechanism better, this yields if too many wrong attempt:
   //    ****FATAL**** YError looping -- quitting now
   fprintf(stderr,"invalid tag.\n");
   ypush_nil();
   }

// shmo_obj_free is automatically called by Yorick to cleanup object instance
// data when object is no longer referenced
static void shmo_obj_free(void *addr) {
   if (debug) printf ("shmo_obj_free\n");
   // destructor has nothing to cleanup
   shmo_object *self = (shmo_object *)addr;
   // self->timestamp;
   // self->data;
   //if (debug) printf ("free data\n");
   //free(self->data);
   }

// shmo_obj_print is used by Yorick's info command
static void shmo_obj_print(void *addr) {
   char s[256];
   if (debug) printf ("shmo_obj_print\n");
   shmo_object *self = (shmo_object *)addr;
   //snprintf(s,256,"%s: frameId %d type %%d dims %%d ts %f",shmo_class.type_name,self->frameId,self->timestamp);
   //y_print(s, 1);
   }

// shmo_obj_eval <--> obj()
// implement a simple dictonary for our purpose here
static void shmo_obj_eval(void *addr, int argc) {
   long index;
   char *key;
   if (debug) printf ("shmo_obj_eval\n");
//    if (argc != 1) {
//       y_error("expecting single key string");
//       }
//    else {
//       key = ygets_q(argc - 1);
//       if (key) {
//          index = yfind_global(key, 0);
//          push_tag((shmo_object *)addr, index);
//          }
//       else {
//          y_error("invalid key");
//          }
//       }
   }

static void shmo_obj_extract(void *addr, long index) {
   if (debug) printf ("shmo_obj_extract index %d key %s\n",index,yfind_name(index));
   if (!strcmp(yfind_name(index),"val")) {
      if (debug) printf("fixme: pushing val\n");
      ypush_nil();
   } else {
      if (debug) printf("invalid element %s",yfind_name(index));
      YError("invalid variable");
   }
}

//- end shmo_object----------------------------------------------



void Y_shmoobj(int nArgs) {
   shmo_object *self;
   
   if (nArgs!=1) YError("shmoobj takes exactly 1 array pointer argument");
   
   Array *array= (Array *)Pointee(yarg_sp(0));
   
   int typeid = array->type.base->dataOps->typeID;
   int countdims = CountDims(array->type.dims);
   long totalnumber = TotalNumber(array->type.dims); // also as, array->type.number

   long bytes =   2 * sizeof(int)                       // typeID + number of dimensions
                + countdims * sizeof(long)              // size of each dimension
                + totalnumber * array->type.base->size; // data

   if (debug) {
      printf ("   element in array %d\n",totalnumber);
      printf ("   CountDims is %d\n",CountDims(array->type.dims));
      printf ("   typeID %d\n",array->type.base->dataOps->typeID);
      printf ("   numbytes %d\n",totalnumber * array->type.base->size);
   }
   
   if (debug) printf("shmoobj constructor\n");
   self = (shmo_object *)ypush_obj(&shmo_class, sizeof(shmo_object));
}

