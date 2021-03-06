#include "rgeos.h"

SEXP rgeos_buffer(SEXP env, SEXP obj, SEXP byid, SEXP id, SEXP width, SEXP quadsegs, 
                  SEXP capStyle, SEXP joinStyle, SEXP mitreLimit) {
    int i;
    GEOSContextHandle_t GEOShandle = getContextHandle(env);
    
    GEOSGeometry* geom = rgeos_convert_R2geos(env, obj);
    SEXP p4s = GET_SLOT(obj, install("proj4string"));
    
    int n;
    if (LOGICAL_POINTER(byid)[0])
        n = GEOSGetNumGeometries_r(GEOShandle, geom);
    else
        n = 1;
    
    GEOSGeometry** geoms = (GEOSGeometry**) R_alloc((size_t) n, sizeof(GEOSGeometry*));
    SEXP newids;
    PROTECT(newids = NEW_CHARACTER(n));
    
    GEOSGeometry* curgeom = geom;
    GEOSGeometry* thisgeom;
    int k = 0;
    for(i=0, k=0; i<n; i++) {
        if ( n > 1) {
            curgeom = (GEOSGeom) GEOSGetGeometryN_r(GEOShandle, geom, i);
            if (curgeom == NULL) error("rgeos_buffer: unable to get subgeometries");
        }
        
        thisgeom = GEOSBufferWithStyle_r(GEOShandle, curgeom, 
                                         NUMERIC_POINTER(width)[i], 
                                         INTEGER_POINTER(quadsegs)[0], 
                                         INTEGER_POINTER(capStyle)[0], 
                                         INTEGER_POINTER(joinStyle)[0],  
                                         NUMERIC_POINTER(mitreLimit)[0]);
// modified 131004 RSB 
// https://stat.ethz.ch/pipermail/r-sig-geo/2013-October/019470.html
        if (!GEOSisEmpty_r(GEOShandle, thisgeom)) {
            geoms[k] = thisgeom;
            SET_STRING_ELT(newids, k, STRING_ELT(id, i));
            k++;
        }

    }

    GEOSGeom_destroy_r(GEOShandle, geom);

    if (k == 0) {
        UNPROTECT(1);
        return(R_NilValue);
    }

    GEOSGeometry* res;
    if (k == 1)
        res = geoms[0];
    else
        res = GEOSGeom_createCollection_r(GEOShandle, GEOS_GEOMETRYCOLLECTION, geoms, (unsigned int) k);

    SEXP ans;
    PROTECT(ans = rgeos_convert_geos2R(env, res, p4s, newids)); // releases res
    UNPROTECT(2);
    return(ans);
}
