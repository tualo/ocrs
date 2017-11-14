
select fast_access_tour.plz,count(*) from fast_access_tour
join
 (
select
bereiche_plz.plz,
bereiche_plz.regiogruppe
 from
bereiche join
bereiche_plz on
bereiche.regiogruppe = bereiche_plz.regiogruppe
and bereiche.name = bereiche_plz.name
 where alleplz=0

and bereiche_plz.regiogruppe='Zustellung'
) b
on b.plz=fast_access_tour.plz
and b.regiogruppe=fast_access_tour.regiogruppe

group by plz
