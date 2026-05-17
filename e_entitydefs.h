/// @brief Entity tags

/* idea is to bit-shift these elements by powers of 2 and OR-ing them together so that we can check for membership. */
typedef enum {
    E_PLAYER,
    E_MONSTER,
    NUMBER_OF_ENTITY_TAGS, /* 2 */
} E_EntityTag;