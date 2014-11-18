/* 
 * File:   field.h
 * Author: akaspar
 *
 * Created on November 18, 2014, 9:46 AM
 */

#ifndef FIELD_H
#define	FIELD_H

#include "../math/iterator2d.h"
#include "../math/mat.h"
#include "../math/point.h"
#include "../im/patch.h"
#include "../im/texture.h"

#include <map>
#include <string>

namespace pm {
    
    template <int RowMajor = true>
    class Field2D : public Iterable2D<Point2i, RowMajor> {
    public:
        
        const union {
            int width;
            int cols;
        };
        
        const union {
            int height;
            int rows;
        };
        
        Field2D(int w, int h) : width(w), height(h){
        }
        
    public:

        /**
         * Class mask (no field) that behaves like a matrix but with a type
         */
        template <typename T>
        struct Entry : public Mat {

            inline const T &at(const Point2i &p) const {
                return at(p.y, p.x);
            }
            inline T &at(const Point2i &p) {
                return at(p.y, p.x);
            }
            inline const T &at(int y, int x) const {
                return *ptr<T>(y, x);
            }
            inline T &at(int y, int x) {
                return *ptr<T>(y, x);
            }

            // default entry constructor
            Entry() : Mat(){}
            // how to make a copy
            Entry(const Mat &f) : Mat(f){
                assert(sizeof(T) == f.elemSize() && "Field entry with invalid element size!");
            }

        protected:
            Entry(int h, int w) : Mat(h, w, sizeof(T), IM_UNKNOWN) {}
        };

        template < typename T >
        FieldEntry<T> createEntry(const std::string &name){
            auto it = entries.find(name);
            assert(it == entries.end() && "Creating field entry that already exists!");

            // create entry and insert
            FieldEntry<T> entry(height, width);
            entries.insert(it, std::pair<std::string, Mat>(name, entry));
            return entry;
        }

        template < typename T >
        FieldEntry<T> getEntry(const std::string &name){
            auto it = entries.find(name);
            assert(it != entries.end() && "Getting a field entry that does not exist!");
            return it->second;
        }
        
        std::vector<Mat> layers() {
            std::vector<Mat> list;
            list.resize(entries.size());
            for(auto k : entries){
                list.push_back(k->second);
            }
            return list;
        }
        
    private:
        std::map<std::string, Mat> entries;
    };
    
}

#endif	/* FIELD_H */

