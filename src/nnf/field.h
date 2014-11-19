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
    
    class DataProvider {
        virtual void load()
    };
    
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
		 * Returns the number of channels needed to store the field data
		 * given the specified data type for storage
		 */
		virtual int dataChannels(DataType type) {
			switch(type){
				case IM_8U:
					return totalChannels<unsigned char>();
				case IM_8S:
					return totalChannels<signed char>();
				case IM_32S:
					return totalChannels<int>();
				case IM_32F:
					return totalChannels<float>();
				case IM_64F:
					return totalChannels<double>();
				default:
					assert(0 && "Unsupported data type for field channels!");
					return 0;
			}
		}
		
		virtual void load(const Point2i &, int, byte* data){
			// no implementation by default
		}
		
		virtual void save(const Point2i &, int, byte* data){
			// no implementation by default
		}

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
            Entry(int h, int w, int ch = 1) : Mat(h, w, sizeof(T), ch) {}
        };

        template < typename T >
        Entry<T> createEntry(const std::string &name){
            auto it = entries.find(name);
            assert(it == entries.end() && "Creating field entry that already exists!");

            // create entry and insert
            Entry<T> entry(height, width);
            entries.insert(it, std::pair<std::string, Mat>(name, entry));
            return entry;
        }

        template < typename T >
        Entry<T> getEntry(const std::string &name){
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
        
        std::vector<const Mat> layers() const {
            std::vector<const Mat> list;
            list.resize(entries.size());
            for(auto k : entries){
                list.push_back(k->second);
            }
            return list;
        }
        
        template < typename T >
        struct EntryLayout {
            
            enum {
                bytesPerChannel = sizeof(T)
            };
            
            int fullChannels;
            int extraBytes;
            
            int totalChannels() const {
                return fullChannels + extraBytes > 0 ? 1 : 0;
            }
            
            EntryLayout(int ch, int eb) : fullChannels(ch), extraBytes(eb){}
        };
        
        template < typename T >
        static EntryLayout<T> layout(const Mat &m) const {
            int bytesPerChannel = sizeof(T);
            int fullChannels = m.elemSize() / bytesPerChannel;
            int extraBytes = m.elemSize() % bytesPerChannel;
            return EntryLayout<T>(fullChannels, extraBytes);
        }
        
        template <typename T>
        int totalChannels() const {
            int channels = 0;
            for(auto it : entries) {
                channels += layout(it->second).totalChannels();
            }
            return channels;
        }
        
        void 
        
    private:
        std::map<std::string, Mat> entries;
    };
    
}

#endif	/* FIELD_H */

