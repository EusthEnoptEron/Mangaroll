package ch.zomg.mangaroll.query;

import com.google.gson.Gson;

/**
 * Created by Simon on 2016/03/30.
 */
public abstract class Fetcher {
    private boolean hasMore;
    private String thumb;
    private String name;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getThumb() {
        return thumb;
    }

    public void setThumb(String thumb) {
        this.thumb = thumb;
    }

    public boolean hasMore() {
        return hasMore;
    }

    public abstract boolean isContainerProvider();

    protected void setHasMore(boolean hasMore) {
        this.hasMore = hasMore;
    }

    public static Fetcher getInitialFetcher(String descriptorString) {
        Gson gson = new Gson();
        Descriptor descriptor = gson.fromJson(descriptorString, Descriptor.class);
        if (descriptor != null) {
            if (descriptor.getType() == Descriptor.Type.CONTAINER) {
                return new ContainerFetcher(descriptor);
            } else {
                return new MangaFetcher(descriptor);
            }
        } else {
            return null;
        }
    }
}
