package ch.zomg.mangaroll.query;

import com.google.gson.Gson;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by Simon on 2016/03/30.
 */
public class ContainerFetcher extends Fetcher {
    private Descriptor descriptor;

    public ContainerFetcher(Descriptor descriptor) {
        this.descriptor = descriptor;
    }


    public Fetcher[] fetch() {
        try {
            Document doc = Jsoup.connect(descriptor.getUrl()).get();

            List<Fetcher> fetchers = new ArrayList<>();

            // Search for items
            for(Element element : doc.select(descriptor.getItemSelector())) {
                String link = element.select(descriptor.getLinkSelector()).attr("href");
                String name = element.select(descriptor.getNameSelector()).text();
//                String thumb = element.select(descriptor.getThumbSelector()).attr("src");

                Descriptor childDescriptor = descriptor.getHandler().clone();
                childDescriptor.setUrl(link);

                if(childDescriptor.getType() == Descriptor.Type.CONTAINER) {
                    ContainerFetcher fetcher = new ContainerFetcher(childDescriptor);
                    fetcher.setName(name);
                    fetchers.add(fetcher);
                } else {
                    MangaFetcher fetcher = new MangaFetcher(childDescriptor);
                    fetcher.setName(name);
                    fetchers.add(fetcher);
                }

                descriptor.getHandler().clone();
            }

            // Determine if there is more
            if(descriptor.getNextPageSelector() != null) {
                Elements nextPageLink = doc.select(descriptor.getNextPageSelector());
                setHasMore(nextPageLink.size() > 0);
                if(hasMore()) {
                    descriptor.setUrl(nextPageLink.get(0).attr("href"));
                }
            } else {
                setHasMore(false);
            }

            return fetchers.toArray(new Fetcher[fetchers.size()]);
        } catch (IOException e) {
            return new Fetcher[0];
        }
    }

    @Override
    public boolean isContainerProvider() {
        return true;
    }
}
