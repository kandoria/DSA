#include <stdlib.h>
00002 #include "collection.h"
00003 #include "trie.h"
00004 #include "mtab.h"
00005 
00006 /*
00007  * allocate a new node
00008  */
00009 cp_trie_node *cp_trie_node_new(void *leaf, cp_mempool *pool) 
00010 { 
00011     cp_trie_node *node;
00012     if (pool) 
00013         node = (cp_trie_node *) cp_mempool_calloc(pool);
00014     else
00015         node = (cp_trie_node *) calloc(1, sizeof(cp_trie_node)); 
00016     if (node) 
00017     { 
00018         node->others = mtab_new(0); 
00019         node->leaf = leaf; 
00020     } 
00021 
00022     return node; 
00023 } 
00024 
00025 /*
00026  * recursively delete the subtree at node
00027  */
00028 void *cp_trie_node_delete(cp_trie *grp, cp_trie_node *node) 
00029 { 
00030     void *leaf = NULL; 
00031 
00032     if (node) 
00033     { 
00034         mtab_delete_custom(node->others, grp, (mtab_dtr)cp_trie_delete_mapping);
00035         leaf = node->leaf; 
00036         if (grp->mempool)
00037             cp_mempool_free(grp->mempool, node);
00038         else
00039             free(node); 
00040     } 
00041 
00042     if (leaf && grp->delete_leaf && grp->mode & COLLECTION_MODE_DEEP)
00043         (*grp->delete_leaf)(leaf);
00044 
00045     return leaf; 
00046 } 
00047 
00048 /* 
00049  * uses the mtab_dtr 'owner' parameter (grp here) to recursively delete 
00050  * sub-nodes
00051  */
00052 void cp_trie_delete_mapping(cp_trie *grp, mtab_node *map_node)
00053 {
00054     if (map_node)
00055     {
00056         if (map_node->attr) free(map_node->attr);
00057         if (map_node->value) cp_trie_node_delete(grp, map_node->value);
00058     }
00059 }
00060 
00061 void cp_trie_node_unmap(cp_trie *grp, cp_trie_node **node) 
00062 { 
00063     cp_trie_node_delete(grp, *node); 
00064     *node = NULL; 
00065 } 
00066 
00067 int cp_trie_lock_internal(cp_trie *grp, int type)
00068 {
00069     int rc = 0;
00070     switch(type)
00071     {
00072         case COLLECTION_LOCK_READ:
00073             rc = cp_lock_rdlock(grp->lock);
00074             break;
00075 
00076         case COLLECTION_LOCK_WRITE:
00077             rc = cp_lock_wrlock(grp->lock);
00078             break;
00079 
00080         case COLLECTION_LOCK_NONE:
00081         default:
00082             break; 
00083     }
00084 
00085     return rc;
00086 }
00087 
00088 int cp_trie_unlock_internal(cp_trie *grp)
00089 {
00090     return cp_lock_unlock(grp->lock);
00091 }
00092 
00093 int cp_trie_txlock(cp_trie *grp, int type)
00094 {
00095     if (grp->mode & COLLECTION_MODE_NOSYNC) return 0;
00096     if (grp->mode & COLLECTION_MODE_IN_TRANSACTION && 
00097         grp->txtype == COLLECTION_LOCK_WRITE)
00098     {
00099         cp_thread self = cp_thread_self();
00100         if (cp_thread_equal(self, grp->txowner)) return 0;
00101     }
00102     return cp_trie_lock_internal(grp, type);
00103 }
00104 
00105 int cp_trie_txunlock(cp_trie *grp)
00106 {
00107     if (grp->mode & COLLECTION_MODE_NOSYNC) return 0;
00108     if (grp->mode & COLLECTION_MODE_IN_TRANSACTION && 
00109         grp->txtype == COLLECTION_LOCK_WRITE)
00110     {
00111         cp_thread self = cp_thread_self();
00112         if (cp_thread_equal(self, grp->txowner)) return 0;
00113     }
00114     return cp_trie_unlock_internal(grp);
00115 }
00116 
00117 /* lock and set the transaction indicators */
00118 int cp_trie_lock(cp_trie *grp, int type)
00119 {
00120     int rc;
00121     if ((grp->mode & COLLECTION_MODE_NOSYNC)) return EINVAL;
00122     if ((rc = cp_trie_lock_internal(grp, type))) return rc;
00123     grp->txtype = type;
00124     grp->txowner = cp_thread_self();
00125     grp->mode |= COLLECTION_MODE_IN_TRANSACTION;
00126     return 0;
00127 }
00128 
00129 /* unset the transaction indicators and unlock */
00130 int cp_trie_unlock(cp_trie *grp)
00131 {
00132     cp_thread self = cp_thread_self();
00133     if (grp->txowner == self)
00134     {
00135         grp->txtype = 0;
00136         grp->txowner = 0;
00137         grp->mode ^= COLLECTION_MODE_IN_TRANSACTION;
00138     }
00139     else if (grp->txtype == COLLECTION_LOCK_WRITE)
00140         return -1;
00141     return cp_trie_unlock_internal(grp);
00142 }
00143 
00144 /* get the current collection mode */
00145 int cp_trie_get_mode(cp_trie *grp)
00146 {
00147     return grp->mode;
00148 }
00149 
00150 /* set mode bits on the trie mode indicator */
00151 int cp_trie_set_mode(cp_trie *grp, int mode)
00152 {
00153     int nosync;
00154 
00155     /* can't set NOSYNC in the middle of a transaction */
00156     if ((grp->mode & COLLECTION_MODE_IN_TRANSACTION) && 
00157         (mode & COLLECTION_MODE_NOSYNC)) return EINVAL;
00158     
00159     nosync = grp->mode & COLLECTION_MODE_NOSYNC;
00160     if (!nosync)
00161         if (cp_trie_txlock(grp, COLLECTION_LOCK_WRITE))
00162             return -1;
00163 
00164     grp->mode |= mode;
00165 
00166     if (!nosync)
00167         cp_trie_txunlock(grp);
00168 
00169     return 0;
00170 }
00171 
00172 /* unset mode bits on the grp mode indicator. if unsetting 
00173  * COLLECTION_MODE_NOSYNC and the grp was not previously synchronized, the 
00174  * internal synchronization structure is initalized.
00175  */
00176 int cp_trie_unset_mode(cp_trie *grp, int mode)
00177 {
00178     int nosync = grp->mode & COLLECTION_MODE_NOSYNC;
00179 
00180     if (!nosync)
00181         if (cp_trie_txlock(grp, COLLECTION_LOCK_WRITE))
00182             return -1;
00183     
00184     /* handle the special case of unsetting COLLECTION_MODE_NOSYNC */
00185     if ((mode & COLLECTION_MODE_NOSYNC) && grp->lock == NULL)
00186     {
00187         /* grp can't be locked in this case, no need to unlock on failure */
00188         if ((grp->lock = malloc(sizeof(cp_lock))) == NULL)
00189             return -1; 
00190         if (cp_lock_init(grp->lock, NULL))
00191             return -1;
00192     }
00193     
00194     /* unset specified bits */
00195     grp->mode &= grp->mode ^ mode;
00196     if (!nosync)
00197         cp_trie_txunlock(grp);
00198 
00199     return 0;
00200 }
00201 
00202 
00203 cp_trie *cp_trie_create_trie(int mode,
00204                              cp_copy_fn copy_leaf,
00205                              cp_destructor_fn delete_leaf) 
00206 { 
00207     cp_trie *grp = calloc(1, sizeof(cp_trie)); 
00208 
00209     if (grp == NULL) return NULL; 
00210     grp->root = cp_trie_node_new(NULL, NULL); //~~ what if mempool set later?
00211     if (grp->root == NULL) 
00212     { 
00213         free(grp); 
00214         return NULL; 
00215     } 
00216 
00217     if (!(mode & COLLECTION_MODE_NOSYNC))
00218     {
00219         if ((grp->lock = malloc(sizeof(cp_lock))) == NULL)
00220         {
00221             cp_trie_node_delete(grp, grp->root);
00222             free(grp);
00223             return NULL;
00224         }
00225 
00226         if (cp_lock_init(grp->lock, NULL)) 
00227         {
00228             free(grp->lock);
00229             cp_trie_node_delete(grp, grp->root);
00230             free(grp); 
00231             return NULL; 
00232         } 
00233     }
00234 
00235     grp->mode = mode;
00236     grp->copy_leaf = copy_leaf;
00237     grp->delete_leaf = delete_leaf;
00238 
00239     return grp; 
00240 } 
00241 
00242 cp_trie *cp_trie_create(int mode)
00243 {
00244     return cp_trie_create_trie(mode, NULL, NULL);
00245 }
00246 
00247 /*
00248  * recursively deletes trie structure
00249  */
00250 int cp_trie_destroy(cp_trie *grp) 
00251 { 
00252     int rc = 0; 
00253 
00254     /* no locking is done here. It is the application's responsibility to
00255      * ensure that the trie isn't being destroyed while it's still in use
00256      * by other threads.
00257      */
00258     if (grp)
00259     { 
00260         cp_trie_node_delete(grp, grp->root); 
00261         if (grp->lock) 
00262         {
00263             rc = cp_lock_destroy(grp->lock); 
00264             free(grp->lock);
00265         }
00266         free(grp); 
00267     } 
00268     else rc = -1; 
00269 
00270     return rc; 
00271 } 
00272 
00273 void *NODE_STORE(cp_trie_node *node, char *key, cp_trie_node *sub)
00274 {
00275     char *k = strdup(key); 
00276     if (k == NULL) return NULL;
00277     return mtab_put(node->others, *k, sub, k);
00278 }
00279 
00280 /*
00281  * since cp_trie compresses single child nodes, the following cases are handled
00282  * here. 'abc' etc denote existing path, X marks the spot:
00283  *
00284  * (1) simple case: abc-X         - first mapping for abc
00285  *
00286  * (2) mid-branch:  ab-X-c        - abc already mapped, but ab isn't 
00287  *
00288  * (3) new branch:  ab-X-cd       - the key abcd is already mapped
00289  *                      \          
00290  *                       ef         the key abef is to be added
00291  *
00292  * (4) extending:   abc-de-X      - abc mapped, abcde isn't
00293  * 
00294  * (5) replace:     abc-X         - abc already mapped, just replace leaf  
00295  */
00296 int cp_trie_node_store(cp_trie *grp, 
00297                        cp_trie_node *node, 
00298                        char *key, 
00299                        void *leaf) 
00300 { 
00301 
00302     char *next;
00303     mtab_node *map_node;
00304     cp_trie_node *sub;
00305 
00306     map_node = NODE_MATCH(node, key); 
00307 
00308     if (map_node == NULL) /* not mapped - just add it */
00309     { 
00310         sub = cp_trie_node_new(leaf, grp->mempool); 
00311         if (NODE_STORE(node, key, sub) == NULL) return -1; 
00312     } 
00313     else
00314     {
00315         next = map_node->attr;
00316         while (*key && *key == *next) 
00317         {
00318             key++;
00319             next++;
00320         }
00321         
00322         if (*next) /* branch abc, key abx or ab */
00323         {
00324             cp_trie_node *old = map_node->value;
00325             if ((sub = cp_trie_node_new(NULL, grp->mempool)) == NULL) return -1;
00326             if (NODE_STORE(sub, next, old) == NULL) return -1;
00327             *next = '\0'; //~~ truncate and realloc - prevent dangling key
00328             map_node->value = sub;
00329             if (*key) /* key abx */
00330             {
00331                 if ((NODE_STORE(sub, key, 
00332                                 cp_trie_node_new(leaf, grp->mempool)) == NULL))
00333                     return -1;
00334             }
00335             else /* key ab */
00336                 sub->leaf = leaf;
00337         }
00338         else if (*key) /* branch abc, key abcde */
00339             return cp_trie_node_store(grp, map_node->value, key, leaf);
00340 
00341         else  /* branch abc, key abc */
00342         {
00343             cp_trie_node *node = ((cp_trie_node *) map_node->value);
00344             if (node->leaf && grp->delete_leaf && 
00345                 grp->mode & COLLECTION_MODE_DEEP)
00346                 (*grp->delete_leaf)(node->leaf);
00347             node->leaf = leaf;
00348         }
00349     }
00350     return 0;
00351 } 
00352 
00353 /*
00354  * wrapper for the recursive insertion - implements collection mode setting
00355  */
00356 int cp_trie_add(cp_trie *grp, char *key, void *leaf) 
00357 { 
00358     int rc = 0; 
00359     if ((rc = cp_trie_txlock(grp, COLLECTION_LOCK_WRITE))) return rc; 
00360 
00361     if ((grp->mode & COLLECTION_MODE_COPY) && grp->copy_leaf && (leaf != NULL))
00362     {
00363         leaf = (*grp->copy_leaf)(leaf);
00364         if (leaf == NULL) goto DONE;
00365     }
00366 
00367     if (key == NULL) /* map NULL key to root node */
00368     {
00369         if (grp->root->leaf && 
00370                 grp->delete_leaf && grp->mode & COLLECTION_MODE_DEEP)
00371             (*grp->delete_leaf)(grp->root->leaf);
00372         grp->root->leaf = leaf; 
00373     }
00374     else 
00375     {
00376         if ((rc = cp_trie_node_store(grp, grp->root, key, leaf)))
00377             goto DONE;
00378     } 
00379 
00380     grp->path_count++; 
00381 
00382 DONE:
00383     cp_trie_txunlock(grp);
00384     return rc; 
00385 } 
00386 
00387 /* helper functions for cp_trie_remove */
00388 
00389 static char *mergestr(char *s1, char *s2)
00390 {
00391     char *s;
00392     int len = strlen(s1) + strlen(s2);
00393 
00394     s = malloc((len + 1) * sizeof(char));
00395     if (s == NULL) return NULL;
00396     
00397 #ifdef CP_HAS_STRLCPY
00398     strlcpy(s, s1, len + 1);
00399 #else
00400     strcpy(s, s1);
00401 #endif /* CP_HAS_STRLCPY */
00402 #ifdef CP_HAS_STRLCAT
00403     strlcat(s, s2, len + 1);
00404 #else
00405     strcat(s, s2);
00406 #endif /* CP_HAS_STRLCAT */
00407 
00408     return s;
00409 }
00410 
00411 static mtab_node *get_single_entry(mtab *t)
00412 {
00413     int i;
00414 
00415     for (i = 0; i < t->size; i++)
00416         if (t->table[i]) return t->table[i];
00417 
00418     return NULL;
00419 }
00420 
00421 /*
00422  * removing mappings, the following cases are possible:
00423  * 
00424  * (1) simple case:       abc-X       removing mapping abc
00425  *
00426  * (2) branch:            abc-de-X    removing mapping abcde -
00427  *                           \        mapping abcfg remains, but
00428  *                            fg      junction node abc no longer needed
00429  *
00430  * (3) mid-branch:        abc-X-de    removing mapping abc - mapping abcde
00431  *                                    remains
00432  */
00433 int cp_trie_remove(cp_trie *grp, char *key, void **leaf)
00434 {
00435     int rc = 0;
00436     cp_trie_node *link = grp->root; 
00437     cp_trie_node *prev = NULL;
00438     char ccurr, cprev = 0;
00439     mtab_node *map_node;
00440     char *branch;
00441 
00442     if (link == NULL) return 0; /* tree is empty */
00443 
00444     if ((rc = cp_trie_txlock(grp, COLLECTION_LOCK_READ))) return rc;
00445  
00446     if (key == NULL) /* as a special case, NULL keys are stored on the root */
00447     {
00448         if (link->leaf)
00449         {
00450             grp->path_count--;
00451             link->leaf = NULL;
00452         }
00453         goto DONE;
00454     }
00455 
00456     /* keep pointers one and two nodes up for merging in cases (2), (3) */
00457     ccurr = *key;
00458     while ((map_node = NODE_MATCH(link, key)) != NULL) 
00459     {
00460         branch = map_node->attr;
00461  
00462         while (*key && *key == *branch)
00463         {
00464             key++;
00465             branch++;
00466         }
00467         if (*branch) break; /* mismatch */
00468         if (*key == '\0') /* found */
00469         {
00470             char *attr;
00471             cp_trie_node *node = map_node->value;
00472             if (leaf) *leaf = node->leaf;
00473             if (node->leaf)
00474             {
00475                 grp->path_count--;
00476                 rc = 1;
00477                 /* unlink leaf */
00478                 if (grp->delete_leaf && grp->mode & COLLECTION_MODE_DEEP)
00479                     (*grp->delete_leaf)(node->leaf);
00480                 node->leaf = NULL;
00481 
00482                 /* now remove node - case (1) */
00483                 if (mtab_count(node->others) == 0) 
00484                 {
00485                     mtab_remove(link->others, ccurr);
00486                     cp_trie_node_unmap(grp, &node);
00487                     
00488                     /* case (2) */
00489                     if (prev &&
00490                         mtab_count(link->others) == 1 && link->leaf == NULL)
00491                     {
00492                         mtab_node *sub2 = mtab_get(prev->others, cprev);
00493                         mtab_node *sub = get_single_entry(link->others);
00494                         attr = mergestr(sub2->attr, sub->attr);
00495                         if (attr)
00496                         {
00497                             if (NODE_STORE(prev, attr, sub->value))
00498                             {
00499                                 mtab_remove(link->others, sub->key);
00500                                 cp_trie_node_delete(grp, link);
00501                             }
00502                             free(attr);
00503                         }
00504                     }
00505                 }
00506                 else if (mtab_count(node->others) == 1) /* case (3) */
00507                 {
00508                     mtab_node *sub = get_single_entry(node->others);
00509                     attr = mergestr(map_node->attr, sub->attr);
00510                     if (attr)
00511                     {
00512                         if (NODE_STORE(link, attr, sub->value))
00513                         {
00514                             mtab_remove(node->others, sub->key);
00515                             cp_trie_node_delete(grp, node);
00516                         }
00517                         free(attr);
00518                     }
00519                 }
00520             }
00521             break;
00522         }
00523         
00524         prev = link;
00525         cprev = ccurr;
00526         ccurr = *key;
00527         link = map_node->value; 
00528     } 
00529 
00530 DONE: 
00531     cp_trie_txunlock(grp);
00532     return rc;
00533 }
00534 
00535 void *cp_trie_exact_match(cp_trie *grp, char *key)
00536 {
00537     void *last = NULL;
00538     cp_trie_node *link = grp->root; 
00539     mtab_node *map_node;
00540     char *branch = NULL;
00541 
00542     if (cp_trie_txlock(grp, COLLECTION_LOCK_READ)) return NULL; 
00543  
00544     while ((map_node = NODE_MATCH(link, key)) != NULL) 
00545     {
00546         branch = map_node->attr;
00547  
00548         while (*key && *key == *branch)
00549         {
00550             key++;
00551             branch++;
00552         }
00553         if (*branch) break; /* mismatch */
00554 
00555         link = map_node->value; 
00556     } 
00557 
00558     if (link) last = link->leaf;
00559 
00560     cp_trie_txunlock(grp);
00561  
00562     if (*key == '\0' && branch && *branch == '\0') 
00563         return last;  /* prevent match on super-string keys */
00564     return NULL;
00565 }
00566 
00567 /* return a vector containing exact match and any prefix matches */
00568 cp_vector *cp_trie_fetch_matches(cp_trie *grp, char *key)
00569 {
00570     int rc;
00571     cp_vector *res = NULL;
00572     cp_trie_node *link = grp->root;
00573     mtab_node *map_node;
00574     char *branch;
00575 
00576     if ((rc = cp_trie_txlock(grp, COLLECTION_LOCK_READ))) return NULL; 
00577  
00578     while ((map_node = NODE_MATCH(link, key)) != NULL) 
00579     {
00580         branch = map_node->attr;
00581 
00582         while (*key && *key == *branch)
00583         {
00584             key++;
00585             branch++;
00586         }
00587         if (*branch) break; /* mismatch */
00588     
00589         link = map_node->value; 
00590         if (link->leaf)
00591         { 
00592             if (res == NULL)
00593             {
00594                 res = cp_vector_create(1);
00595                 if (res == NULL) break;
00596             }
00597             cp_vector_add_element(res, link->leaf);
00598         } 
00599     }
00600 
00601     cp_trie_txunlock(grp);
00602     return res; 
00603 } 
00604 
00605 static void extract_subtrie(cp_trie_node *link, cp_vector *v);
00606 
00607 static int extract_node(void *n, void *v)
00608 {
00609     mtab_node *node = n;
00610 
00611     extract_subtrie(node->value, v);
00612 
00613     return 0;
00614 }
00615 
00616 static void extract_subtrie(cp_trie_node *link, cp_vector *v)
00617 {
00618     if (link->leaf)
00619         cp_vector_add_element(v, link->leaf);
00620 
00621     if (link->others)
00622         mtab_callback(link->others, extract_node, v);
00623 }
00624 
00625 /* return a vector containing all entries in subtree under path given by key */
00626 cp_vector *cp_trie_submatch(cp_trie *grp, char *key)
00627 {
00628     int rc;
00629     cp_vector *res = NULL;
00630     cp_trie_node *link = grp->root;
00631     mtab_node *map_node;
00632     char *branch;
00633 
00634     if ((rc = cp_trie_txlock(grp, COLLECTION_LOCK_READ))) return NULL; 
00635  
00636     while ((map_node = NODE_MATCH(link, key)) != NULL) 
00637     {
00638         branch = map_node->attr;
00639 
00640         while (*key && *key == *branch)
00641         {
00642             key++;
00643             branch++;
00644         }
00645 
00646         if (*branch && *key) break; /* mismatch */
00647     
00648         link = map_node->value; 
00649 
00650         if (*key == '\0')
00651         {
00652             res = cp_vector_create(1);
00653             extract_subtrie(link, res);
00654             break;
00655         }
00656     }
00657 
00658     cp_trie_txunlock(grp);
00659     return res; 
00660 } 
00661 
00662 /* return longest prefix match for given key */
00663 int cp_trie_prefix_match(cp_trie *grp, char *key, void **leaf)
00664 { 
00665     int rc, match_count = 0; 
00666     void *last = grp->root->leaf; 
00667     cp_trie_node *link = grp->root; 
00668     mtab_node *map_node;
00669     char *branch;
00670 
00671     if ((rc = cp_trie_txlock(grp, COLLECTION_LOCK_READ))) return rc; 
00672  
00673     while ((map_node = NODE_MATCH(link, key)) != NULL) 
00674     { 
00675         branch = map_node->attr;
00676  
00677         while (*key && *key == *branch)
00678         {
00679             key++;
00680             branch++;
00681         }
00682         if (*branch) break; /* mismatch */
00683 
00684         link = map_node->value; 
00685         if (link->leaf)
00686         { 
00687             last = link->leaf; 
00688             match_count++; 
00689         } 
00690     } 
00691 
00692     *leaf = last; 
00693  
00694     cp_trie_txunlock(grp);
00695  
00696     return match_count; 
00697 } 
00698 
00699 int cp_trie_count(cp_trie *grp)
00700 {
00701     return grp->path_count;
00702 }
00703 
00704 void cp_trie_set_root(cp_trie *grp, void *leaf) 
00705 { 
00706     grp->root->leaf = leaf; 
00707 } 
00708 
00709 /* dump trie to stdout - for debugging */
00710 
00711 static int node_count;
00712 static int depth_total;
00713 static int max_level;
00714 
00715 void cp_trie_dump_node(cp_trie_node *node, int level, char *prefix)
00716 {
00717     int i;
00718     mtab_node *map_node;
00719 
00720     node_count++;
00721     depth_total += level;
00722     if (level > max_level) max_level = level;
00723 
00724     for (i = 0; i < node->others->size; i++)
00725     {
00726         map_node = node->others->table[i];
00727         while (map_node)
00728         {
00729             cp_trie_dump_node(map_node->value, level + 1, map_node->attr);
00730             map_node = map_node->next;
00731         }
00732     }
00733 
00734     for (i = 0; i < level; i++) printf("\t");
00735 
00736     printf(" - %s => [%s]\n", prefix, node->leaf ? (char *) node->leaf : "");
00737 }
00738             
00739 void cp_trie_dump(cp_trie *grp)
00740 {
00741     node_count = 0;
00742     depth_total = 0;
00743     max_level = 0;
00744     
00745     cp_trie_dump_node(grp->root, 0, "");
00746 
00747 #ifdef DEBUG
00748     printf("\n %d nodes, %d deepest, avg. depth is %.2f\n\n", 
00749             node_count, max_level, (float) depth_total / node_count);
00750 #endif
00751 }
00752 
00753 /* set trie to use given mempool or allocate a new one if pool is NULL */
00754 int cp_trie_use_mempool(cp_trie *trie, cp_mempool *pool)
00755 {
00756     int rc = 0;
00757     
00758     if ((rc = cp_trie_txlock(trie, COLLECTION_LOCK_WRITE))) return rc;
00759     
00760     if (pool)
00761     {
00762         if (pool->item_size < sizeof(cp_trie_node))
00763         {
00764             rc = EINVAL;
00765             goto DONE;
00766         }
00767         if (trie->mempool) 
00768         {
00769             if (trie->path_count) 
00770             {
00771                 rc = ENOTEMPTY;
00772                 goto DONE;
00773             }
00774             cp_mempool_destroy(trie->mempool);
00775         }
00776         cp_mempool_inc_refcount(pool);
00777         trie->mempool = pool;
00778     }
00779     else
00780     {
00781         trie->mempool = 
00782             cp_mempool_create_by_option(COLLECTION_MODE_NOSYNC, 
00783                                         sizeof(cp_trie_node), 0);
00784         if (trie->mempool == NULL) 
00785         {
00786             rc = ENOMEM;
00787             goto DONE;
00788         }
00789     }
00790 
00791 DONE:
00792     cp_trie_txunlock(trie);
00793     return rc;
00794 }
00795 
00796 
00797 /* set trie to use a shared memory pool */
00798 int cp_trie_share_mempool(cp_trie *trie, cp_shared_mempool *pool)
00799 {
00800     int rc;
00801 
00802     if ((rc = cp_trie_txlock(trie, COLLECTION_LOCK_WRITE))) return rc;
00803 
00804     if (trie->mempool)
00805     {
00806         if (trie->path_count)
00807         {
00808             rc = ENOTEMPTY;
00809             goto DONE;
00810         }
00811 
         cp_mempool_destroy(trie->mempool);
     }
 
     trie->mempool = cp_shared_mempool_register(pool, sizeof(cp_trie_node));
     if (trie->mempool == NULL) 
     {
         rc = ENOMEM;
         goto DONE;
     }
     
 DONE:
     cp_trie_txunlock(trie);
     return rc;
 }

00827 