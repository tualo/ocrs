DROP TABLE  IF EXISTS faststreethash_complex;
create table faststreethash_complex (
id varchar(50) primary key, date_added datetime ,adr text,strasse varchar(255) ) engine myisam;
insert into faststreethash_complex (id,date_added,strasse,plz,adr)
  select
    md5(strasse) id,
    now(),
    strasse,
    strasse
  from
    fast_access_tour
  where
    regiogruppe='Zustellung'
  group by strasse;
create fulltext index id_ft_faststreethash_complex on faststreethash_complex(adr);
create index id_ft_faststreethash_complex_oplz on faststreethash_complex(plz);





  SELECT
    strasse,
    LEVENSHTEIN_RATIO('Fnednch Eberl Str',adr) lvrval
  FROM (
    SELECT
      strasse,
      match(adr) against('Fnednch Eberl Str') as rel
    FROM faststreethash_complex
    order by rel desc
  ) b
  order by lvrval desc



SELECT
  strasse,
  match(adr) against('Fnednch Eberl Str') as rel
FROM faststreethash_complex
