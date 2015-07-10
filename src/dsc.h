//-----------------------------------------------------------------------------
// Data structures used frequently in the program, various kinds of vectors
// (of real numbers, not symbolic algebra stuff) and our templated lists.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#ifndef __DSC_H
#define __DSC_H

#include "solvespace.h"

class Vector;
class Vector4;
class Point2d;
class hEntity;
class hParam;

class Quaternion {
public:
    // a + (vx)*i + (vy)*j + (vz)*k
    double w, vx, vy, vz;

    static const Quaternion IDENTITY;

    static Quaternion From(double w, double vx, double vy, double vz);
    static Quaternion From(hParam w, hParam vx, hParam vy, hParam vz);
    static Quaternion From(Vector u, Vector v);
    static Quaternion From(Vector axis, double dtheta);

    Quaternion Plus(Quaternion b);
    Quaternion Minus(Quaternion b);
    Quaternion ScaledBy(double s);
    double Magnitude(void);
    Quaternion WithMagnitude(double s);

    // Call a rotation matrix [ u' v' n' ]'; this returns the first and
    // second rows, where that matrix is generated by this quaternion
    Vector RotationU(void);
    Vector RotationV(void);
    Vector RotationN(void);
    Vector Rotate(Vector p);

    Quaternion ToThe(double p);
    Quaternion Inverse(void);
    Quaternion Times(Quaternion b);
    Quaternion Mirror(void);
};

class Vector {
public:
    double x, y, z;

    static Vector From(double x, double y, double z);
    static Vector From(hParam x, hParam y, hParam z);
    static Vector AtIntersectionOfPlanes(Vector n1, double d1,
                                         Vector n2, double d2);
    static Vector AtIntersectionOfLines(Vector a0, Vector a1,
                                        Vector b0, Vector b1,
                                        bool *skew,
                                        double *pa=NULL, double *pb=NULL);
    static Vector AtIntersectionOfPlaneAndLine(Vector n, double d,
                                               Vector p0, Vector p1,
                                               bool *parallel);
    static Vector AtIntersectionOfPlanes(Vector na, double da,
                                         Vector nb, double db,
                                         Vector nc, double dc, bool *parallel);
    static void ClosestPointBetweenLines(Vector pa, Vector da,
                                         Vector pb, Vector db,
                                         double *ta, double *tb);

    double Element(int i);
    bool Equals(Vector v, double tol=LENGTH_EPS);
    bool EqualsExactly(Vector v);
    Vector Plus(Vector b);
    Vector Minus(Vector b);
    Vector Negated(void);
    Vector Cross(Vector b);
    double DirectionCosineWith(Vector b);
    double Dot(Vector b);
    Vector Normal(int which);
    Vector RotatedAbout(Vector orig, Vector axis, double theta);
    Vector RotatedAbout(Vector axis, double theta);
    Vector DotInToCsys(Vector u, Vector v, Vector n);
    Vector ScaleOutOfCsys(Vector u, Vector v, Vector n);
    double DistanceToLine(Vector p0, Vector dp);
    bool OnLineSegment(Vector a, Vector b, double tol=LENGTH_EPS);
    Vector ClosestPointOnLine(Vector p0, Vector dp);
    double Magnitude(void);
    double MagSquared(void);
    Vector WithMagnitude(double s);
    Vector ScaledBy(double s);
    Vector ProjectInto(hEntity wrkpl);
    Vector ProjectVectorInto(hEntity wrkpl);
    double DivPivoting(Vector delta);
    Vector ClosestOrtho(void);
    void MakeMaxMin(Vector *maxv, Vector *minv);
    Vector ClampWithin(double minv, double maxv);
    static bool BoundingBoxesDisjoint(Vector amax, Vector amin,
                                      Vector bmax, Vector bmin);
    static bool BoundingBoxIntersectsLine(Vector amax, Vector amin,
                                          Vector p0, Vector p1, bool segment);
    bool OutsideAndNotOn(Vector maxv, Vector minv);
    Vector InPerspective(Vector u, Vector v, Vector n,
                         Vector origin, double cameraTan);
    Point2d Project2d(Vector u, Vector v);
    Point2d ProjectXy(void);
    Vector4 Project4d(void);
};

class Vector4 {
public:
    double w, x, y, z;

    static Vector4 From(double w, double x, double y, double z);
    static Vector4 From(double w, Vector v3);
    static Vector4 Blend(Vector4 a, Vector4 b, double t);

    Vector4 Plus(Vector4 b);
    Vector4 Minus(Vector4 b);
    Vector4 ScaledBy(double s);
    Vector PerspectiveProject(void);
};

class Point2d {
public:
    double x, y;

    static Point2d From(double x, double y);

    Point2d Plus(Point2d b);
    Point2d Minus(Point2d b);
    Point2d ScaledBy(double s);
    double DivPivoting(Point2d delta);
    double Dot(Point2d p);
    double DistanceTo(Point2d p);
    double DistanceToLine(Point2d p0, Point2d dp, bool segment);
    double Magnitude(void);
    double MagSquared(void);
    Point2d WithMagnitude(double v);
    Point2d Normal(void);
    bool Equals(Point2d v, double tol=LENGTH_EPS);
};

// A simple list
template <class T>
class List {
public:
    T   *elem;
    int  n;
    int  elemsAllocated;

    void AllocForOneMore(void) {
        if(n >= elemsAllocated) {
            elemsAllocated = (elemsAllocated + 32)*2;
            elem = (T *)MemRealloc(elem, (size_t)elemsAllocated*sizeof(elem[0]));
        }
    }

    void Add(T *t) {
        AllocForOneMore();
        elem[n++] = *t;
    }

    void AddToBeginning(T *t) {
        AllocForOneMore();
        memmove(elem+1, elem, n*sizeof(elem[0]));
        n++;
        elem[0] = *t;
    }

    T *First(void) {
        return (n == 0) ? NULL : &(elem[0]);
    }
    T *NextAfter(T *prev) {
        if(!prev) return NULL;
        if(prev - elem == (n - 1)) return NULL;
        return prev + 1;
    }

    void ClearTags(void) {
        int i;
        for(i = 0; i < n; i++) {
            elem[i].tag = 0;
        }
    }

    void Clear(void) {
        if(elem) MemFree(elem);
        elem = NULL;
        n = elemsAllocated = 0;
    }

    void RemoveTagged(void) {
        int src, dest;
        dest = 0;
        for(src = 0; src < n; src++) {
            if(elem[src].tag) {
                // this item should be deleted
            } else {
                if(src != dest) {
                    elem[dest] = elem[src];
                }
                dest++;
            }
        }
        n = dest;
        // and elemsAllocated is untouched, because we didn't resize
    }

    void RemoveLast(int cnt) {
        if(n < cnt) oops();
        n -= cnt;
        // and elemsAllocated is untouched, same as in RemoveTagged
    }

    void Reverse(void) {
        int i;
        for(i = 0; i < (n/2); i++) {
            SWAP(T, elem[i], elem[(n-1)-i]);
        }
    }
};

// A list, where each element has an integer identifier. The list is kept
// sorted by that identifier, and items can be looked up in log n time by
// id.
template <class T, class H>
class IdList {
public:
    T     *elem;
    int   n;
    int   elemsAllocated;

    uint32_t MaximumId(void) {
        uint32_t id = 0;

        int i;
        for(i = 0; i < n; i++) {
            id = max(id, elem[i].h.v);
        }
        return id;
    }

    H AddAndAssignId(T *t) {
        t->h.v = (MaximumId() + 1);
        Add(t);

        return t->h;
    }

    void Add(T *t) {
        if(n >= elemsAllocated) {
            elemsAllocated = (elemsAllocated + 32)*2;
            elem = (T *)MemRealloc(elem, (size_t)elemsAllocated*sizeof(elem[0]));
        }

        int first = 0, last = n;
        // We know that we must insert within the closed interval [first,last]
        while(first != last) {
            int mid = (first + last)/2;
            H hm = elem[mid].h;
            if(hm.v > t->h.v) {
                last = mid;
            } else if(hm.v < t->h.v) {
                first = mid + 1;
            } else {
                dbp("can't insert in list; is handle %d not unique?", t->h.v);
                oops();
            }
        }
        int i = first;

        memmove(elem+i+1, elem+i, (size_t)(n-i)*sizeof(elem[0]));
        elem[i] = *t;
        n++;
    }

    T *FindById(H h) {
        T *t = FindByIdNoOops(h);
        if(!t) {
            dbp("failed to look up item %08x, searched %d items", h.v, n);
            oops();
        }
        return t;
    }

    T *FindByIdNoOops(H h) {
        int first = 0, last = n-1;
        while(first <= last) {
            int mid = (first + last)/2;
            H hm = elem[mid].h;
            if(hm.v > h.v) {
                last = mid-1; // and first stays the same
            } else if(hm.v < h.v) {
                first = mid+1; // and last stays the same
            } else {
                return &(elem[mid]);
            }
        }
        return NULL;
    }

    T *First(void) {
        return (n == 0) ? NULL : &(elem[0]);
    }
    T *NextAfter(T *prev) {
        if(!prev) return NULL;
        if(prev - elem == (n - 1)) return NULL;
        return prev + 1;
    }

    void ClearTags(void) {
        int i;
        for(i = 0; i < n; i++) {
            elem[i].tag = 0;
        }
    }

    void Tag(H h, int tag) {
        int i;
        for(i = 0; i < n; i++) {
            if(elem[i].h.v == h.v) {
                elem[i].tag = tag;
            }
        }
    }

    void RemoveTagged(void) {
        int src, dest;
        dest = 0;
        for(src = 0; src < n; src++) {
            if(elem[src].tag) {
                // this item should be deleted
            } else {
                if(src != dest) {
                    elem[dest] = elem[src];
                }
                dest++;
            }
        }
        n = dest;
        // and elemsAllocated is untouched, because we didn't resize
    }
    void RemoveById(H h) {
        ClearTags();
        FindById(h)->tag = 1;
        RemoveTagged();
    }

    void MoveSelfInto(IdList<T,H> *l) {
        memcpy(l, this, sizeof(*this));
        elemsAllocated = n = 0;
        elem = NULL;
    }

    void DeepCopyInto(IdList<T,H> *l) {
        l->elem = (T *)MemAlloc(elemsAllocated * sizeof(elem[0]));
        memcpy(l->elem, elem, elemsAllocated * sizeof(elem[0]));
        l->elemsAllocated = elemsAllocated;
        l->n = n;
    }

    void Clear(void) {
        for(int i = 0; i < n; i++) {
            elem[i].Clear();
        }
        elemsAllocated = n = 0;
        if(elem) MemFree(elem);
        elem = NULL;
    }

};

class NameStr {
public:
    char str[64];

    inline void strcpy(const char *in) {
        memcpy(str, in, min(strlen(in)+1, sizeof(str)));
        str[sizeof(str)-1] = '\0';
    }
};

class BandedMatrix {
public:
    enum {
        MAX_UNKNOWNS   = 16,
        RIGHT_OF_DIAG  = 1,
        LEFT_OF_DIAG   = 2
    };

    double A[MAX_UNKNOWNS][MAX_UNKNOWNS];
    double B[MAX_UNKNOWNS];
    double X[MAX_UNKNOWNS];
    int n;

    void Solve(void);
};

#define RGBi(r, g, b) RgbaColor::From((r), (g), (b))
#define RGBf(r, g, b) RgbaColor::FromFloat((float)(r), (float)(g), (float)(b))

// Note: sizeof(class RgbaColor) should be exactly 4
//
class RgbaColor {
public:
    uint8_t red, green, blue, alpha;

    float redF(void)   const { return (float)red   / 255.0f; }
    float greenF(void) const { return (float)green / 255.0f; }
    float blueF(void)  const { return (float)blue  / 255.0f; }
    float alphaF(void) const { return (float)alpha / 255.0f; }

    bool Equals(RgbaColor c) const {
        return
            c.red   == red   &&
            c.green == green &&
            c.blue  == blue  &&
            c.alpha == alpha;
    }

    uint32_t ToPackedInt(void) const {
        return
            red |
            (uint32_t)(green << 8) |
            (uint32_t)(blue << 16) |
            (uint32_t)((255 - alpha) << 24);
    }

    static RgbaColor From(int r, int g, int b, int a = 255) {
        RgbaColor c;
        c.red   = (uint8_t)r;
        c.green = (uint8_t)g;
        c.blue  = (uint8_t)b;
        c.alpha = (uint8_t)a;
        return c;
    }

    static RgbaColor FromFloat(float r, float g, float b, float a = 1.0) {
        return From(
            (int)(255.1f * r),
            (int)(255.1f * g),
            (int)(255.1f * b),
            (int)(255.1f * a));
    }

    static RgbaColor FromPackedInt(uint32_t bgra) {
        return From(
            (int)((bgra)       & 0xff),
            (int)((bgra >> 8)  & 0xff),
            (int)((bgra >> 16) & 0xff),
            (int)(255 - ((bgra >> 24) & 0xff)));
    }
};

#endif
