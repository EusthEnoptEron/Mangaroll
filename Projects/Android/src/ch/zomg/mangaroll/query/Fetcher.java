package ch.zomg.mangaroll.query;

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

    protected void setHasMore(boolean hasMore) {
        this.hasMore = hasMore;
    }
}
