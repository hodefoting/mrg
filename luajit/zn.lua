-- ffi lua binding for microraptor gui by Øyvind Kolås, public domain
--
--
local ffi = require('ffi')
local C = ffi.load('zn')
local M = setmetatable({C=C},{__index = C})

ffi.cdef[[
typedef struct _Zn Zn;
Zn *            zn_new (const char *db_path);
void            zn_destroy (Zn *zn);
typedef int64_t ZnId;
typedef int64_t ZnSize;
ZnId        zn_string     (Zn *zn, const char *string);             
ZnSize      zn_get_length (Zn *zn, ZnId id);
const void *zn_load      (Zn *zn, ZnId id);
const char *zn_loadz     (Zn *zn, ZnId id);                
void zn_unref            (Zn *zn, ZnId id);           
void zn_ref              (Zn *zn, ZnId id);    
ZnId zn_data             (Zn *zn, const void *data, ZnSize length);
void zn_compute_hash     (Zn *zn, const char *data, ZnSize length, uint8_t *hash);
void  zn_hash            (Zn *zn, ZnId id, uint8_t *hash_buf);
int   zn_hash_length     (Zn *zn);
char *zn_get_hex         (Zn *zn, ZnId id);
char *zn_get_base64      (Zn *zn, ZnId id);
ZnId zn_resolve          (Zn *zn, const uint8_t *hash_bin);
ZnId zn_resolve_hex      (Zn *zn, const char    *hash_hex);
ZnId zn_resolve_base64   (Zn *zn, const char    *hash_base64);
ZnId zn_handle           (Zn *zn); 
void zn_set_key          (Zn *zn, ZnId id, ZnId key, ZnId value);
ZnId zn_get_key          (Zn *zn, ZnId id, ZnId key);
int  zn_has_key          (Zn *zn, ZnId id, ZnId key);
void zn_unset_key        (Zn *zn, ZnId id, ZnId key);
ZnId * zn_list_keys      (Zn *zn, ZnId id);
int    zn_count_keys     (Zn *zn, ZnId id); 
void    zn_set_key_int   (Zn *zn, ZnId id, ZnId key, int64_t value);
int64_t zn_get_key_int   (Zn *zn, ZnId id, ZnId key);
void    zn_set_key_float (Zn *zn, ZnId id, ZnId key, double value);
double  zn_get_key_float (Zn *zn, ZnId id, ZnId key); 
int    zn_set_value      (Zn *zn, ZnId id, ZnId key, int value_no, ZnId value);
void   zn_replace_value  (Zn *zn, ZnId id, ZnId key, int value_no, ZnId new_value);
ZnId   zn_get_value      (Zn *zn, ZnId id, ZnId key, int no); 
ZnId * zn_list_values    (Zn *zn, ZnId id, ZnId key);
int    zn_count_values   (Zn *zn, ZnId id, ZnId key);
/*a shortcut method for passing -1 to set_value's no argument                */

int  zn_append_value     (Zn *zn, ZnId id, ZnId key, ZnId value);              /*

a shortcut method for passing -2 to set_value's no argument, this treats the
key as a set where each value can only occur once                          */

/*/
int  zn_add_value (Zn *zn, ZnId id, ZnId key, ZnId value);                 /*
void zn_unset_value_no  (Zn *zn, ZnId id, ZnId key, int no); 
void zn_unset_value     (Zn *zn, ZnId id, ZnId key, ZnId value);
void zn_unset_value_all (Zn *zn, ZnId id, ZnId key, ZnId value); 
void zn_swap_values(Zn *zn,ZnId id, ZnId key, int value_no, int other_value_no);
ZnId   zn_get_attribute   (Zn *zn, ZnId id, ZnId key, int no, ZnId attribute);
int    zn_has_attribute   (Zn *zn, ZnId id, ZnId key, int no, ZnId attribute);
void   zn_set_attribute   (Zn *zn, ZnId id, ZnId key,
                           int value_no, ZnId attribute, ZnId detail);
void   zn_unset_attribute (Zn *zn, ZnId id, ZnId key, int no, ZnId attribute);
void   zn_set_detail      (Zn *zn, ZnId id, ZnId key, int value_no,
                           ZnId attribute, int detail_no, ZnId detail);
ZnId   zn_get_detail      (Zn *zn, ZnId id, ZnId key,
                           int value_no, ZnId attribute, int detail_no);
void   zn_unset_detail    (Zn *zn, ZnId id, ZnId key, int value_no,
                           ZnId attribute, ZnId detail);
void   zn_unset_detail_all(Zn *zn, ZnId id, ZnId key, int value_no,
                           ZnId attribute, ZnId detail);
void   zn_unset_detail_no (Zn *zn, ZnId id, ZnId key, int value_no,
                           ZnId attribute, int detail_no);  /* XXX:NYI */
void   zn_swap_details    (Zn *zn, ZnId id, ZnId key, int value_no,
                           ZnId attribute, int detail1, int detail2);/*XXX:NYI */
void   zn_replace_detail  (Zn *zn, ZnId id, ZnId key, int value_no,
                           ZnId attribute, int detail_no, ZnId detail);
ZnId * zn_list_attributes (Zn *zn, ZnId id, ZnId key, int value_no);
ZnId * zn_list_details     (Zn *zn, ZnId id, ZnId key, int value_no,
                            ZnId attribute);
int    zn_count_attributes (Zn *zn, ZnId id, ZnId key, int value_no);
int    zn_count_details    (Zn *zn, ZnId id, ZnId key, int value_no,
                            ZnId attribute);                               /* 
The following functions are wrappers for various accessors that provide
C types instead of strings, the underlying stored values are still
strings, these methos are able to parse RDF suffixed ^^ types.             */

void    zn_set_attribute_int (Zn *zn, ZnId id, ZnId key,
                              int value_no, ZnId attribute, int64_t value);
int64_t zn_get_attribute_int (Zn *zn, ZnId id, ZnId key,
                               int value_no, ZnId attribute);
void    zn_set_attribute_float (Zn *zn, ZnId id, ZnId key,
                                int value_no, ZnId attribute, double value);
double  zn_get_attribute_float (Zn *zn, ZnId id, ZnId key,
                                int value_no, ZnId attribute);
void    zn_set_detail_int  (Zn *zn, ZnId id, ZnId key, int value_no,
                           ZnId attribute, int detail_no, int64_t detail);
int64_t zn_get_detail_int  (Zn *zn, ZnId id, ZnId key,
                            int value_no, ZnId attribute, int no);
void zn_flush (Zn *zn);
void        zn_set_salt (Zn *zn, const char *salt); /* XXX: should add length, optionally -1 */
const char *zn_get_salt (Zn *zn);
void zn_set_compress     (Zn *zn, int compression_enabled);
int  zn_get_compress     (Zn *zn);
void zn_set_secret    (Zn *zn, const uint8_t *secret, int length); 
void zn_add_host    (Zn *zn, const char *path);
void zn_remove_host (Zn *zn, const char *path);
const char *zn_get_mime_type    (Zn *zn, ZnId id);
ZnId zn_mktime (Zn *zn,
                int year, int mon, int day,
                int hour, int min, int sec,
                int isdst);
ZnId     zn_time_now            (Zn *zn);
uint64_t zn_get_time_stamp      (Zn *zn, ZnId id);
uint64_t zn_get_prev_time_stamp (Zn *zn, ZnId id);
ZnId     zn_time_current        (Zn *zn); 
ZnId     zn_time_plus           (Zn *zn, ZnId time, uint64_t delta_seconds);
ZnSize zn_entries  (Zn *zn);

/****** XXX XXX  the following should be implemented in straight lua!! */

 /*

The following API is a wrapper API around the above using the keys "children"
and "parents" for managing the associations. This is only a thin layer above
zn - but it provides a useful basis to build further datastructures. All 22!
should probably be removed, using this API instead does provide for shorter C
code - perhaps a set of #defines that even remove the need to pass in zn is a
better strategy - without polluting the library itself?.. one should probably
avoid relying too much on this API.. Even if continuing to use the
datastructures created by it..                                             */

void   zn_remove_children       (Zn *zn, ZnId id);
void   zn_add_child_at          (Zn *zn, ZnId id, int pos, ZnId child);
void   zn_remove_child          (Zn *zn, ZnId id, int pos);
void   zn_replace_child         (Zn *zn, ZnId id, int no, ZnId child);
void   zn_swap_children         (Zn *zn, ZnId id, int pos1, int pos2);
ZnId   zn_get_child             (Zn *zn, ZnId id, int pos);
ZnId  *zn_list_children         (Zn *zn, ZnId id);
int    zn_count_children        (Zn *zn, ZnId id);
void   zn_child_unset_key       (Zn *zn, ZnId id, int no, ZnId key);
ZnId * zn_child_get_key         (Zn *zn, ZnId id, int no, ZnId key);
int    zn_child_has_key         (Zn *zn, ZnId id, int no, ZnId key);
void   zn_child_add_key         (Zn *zn, ZnId id, int no, ZnId key, ZnId value);
void   zn_child_set_key         (Zn *zn, ZnId id, int no, ZnId key, ZnId value);
void   zn_child_unset_key_value (Zn *zn, ZnId id, int no, ZnId key, ZnId value);
ZnId * zn_child_get_keys        (Zn *zn, ZnId id, int no);
int    zn_child_count_keys      (Zn *zn, ZnId id, int no);
ZnId * zn_child_list_keys       (Zn *zn, ZnId id, int no);
ZnId   zn_child_get_key_one     (Zn *zn, ZnId id, int no, ZnId key);
ZnId   zn_child_get_key_one_int (Zn *zn, ZnId id, int no, ZnId key);
void   zn_append_child          (Zn *zn, ZnId id, ZnId child);
ZnId  *zn_get_parents           (Zn *zn, ZnId id);
int    zn_count_parents         (Zn *zn, ZnId id);

]]

function M.new(path)
  local zn = ffi.gc(C.zn_new(path),C.zn_destroy)
  lyd:audio_init "auto"
  return zn
end

ffi.metatype('Zn', {__index = {
  -- maybe we should add a _full version to this as well, then all callback
  -- things in the code would be lifecycle managed.
  note       = function (...) C.lyd_note(...) end,
  audio_init = function (...) C.lyd_audio_init(...) end,
  set_sample_rate = function (...) C.lyd_set_sample_rate(...) end,
  get_sample_rate = function (...) return C.lyd_get_sample_rate(...) end,
  set_patch  = function (...) C.lyd_set_patch(...) end,
  kill = function (...) C.lyd_kill(...) end,

  set_var_handler = function (lyd, cb, data)
    -- manually cast and destroy resources held by lua/C binding
    local notify_fun, cb_fun;
    local notify_cb = function (finalize_data)
      cb_fun:free();
      notify_fun:free();
      return 0;
    end

    cb_fun = ffi.cast ("void (*)(Lyd *, const char *, double , void *)", cb)
    notify_fun = ffi.cast ("void (*)(void*)", notify_cb)
    return C.lyd_set_var_handler_full (lyd, cb_fun, data, notify_fun, NULL)
  end,
  get_patch  = function (...) return C.lyd_get_patch(...) end,
  voice_new = function (...) return C.lyd_voice_new(...) end,
  compile = function (...) return ffi.gc(C.lyd_compile(...), C.lyd_program_free) end
}})

return M
