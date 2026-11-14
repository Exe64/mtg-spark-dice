/* Every language table is complete and interchangeable with the first one. */
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "strings.h"

static const char *const ID[STR_COUNT] = {
#define X(id) #id,
    UI_STRINGS(X)
#undef X
};

/* the printf conversions of a string, in order: "T%d | J%d" -> "dd" */
static void conversions(const char *s, char *out, size_t n)
{
    size_t k = 0;
    for (; *s; s++) {
        if (*s != '%')
            continue;
        s++;
        if (*s == '%')
            continue;
        while (*s && !strchr("dsuxc", *s)) /* flags, width */
            s++;
        assert(*s && k + 1 < n);
        out[k++] = *s;
    }
    out[k] = '\0';
}

int main(void)
{
    for (int l = 0; l < UI_LANG_COUNT; l++) {
        const ui_lang_t *L = ui_langs[l];
        assert(L && L->name && L->name[0]);
        for (int i = 0; i < STR_COUNT; i++) {
            char ref[8], got[8];
            if (!L->str[i] || !L->str[i][0]) {
                fprintf(stderr, "%s: STR_%s is missing\n", L->name, ID[i]);
                return 1;
            }
            conversions(ui_langs[0]->str[i], ref, sizeof(ref));
            conversions(L->str[i], got, sizeof(got));
            if (strcmp(ref, got)) {
                fprintf(stderr, "%s: STR_%s takes \"%s\", %s takes \"%s\"\n", L->name, ID[i], got, ui_langs[0]->name, ref);
                return 1;
            }
            bool fmt = strlen(ID[i]) > 4 && !strcmp(ID[i] + strlen(ID[i]) - 4, "_FMT");
            assert(fmt == (ref[0] != '\0')); /* formats are named _FMT, and nothing else holds a conversion */
        }
        assert(L->n_noun >= 8 && L->n_adj >= 8);
        for (int a = 0; a < L->n_noun; a++)
            for (int b = 0; b < L->n_adj; b++)
                assert(strlen(L->noun[a]) + 1 + strlen(L->adj[b]) <= 23); /* sd_settings_t.name */
    }
    printf("strings: %d strings in %d languages\n", STR_COUNT, UI_LANG_COUNT);
    return 0;
}
